#include "../Inc/Raytracer.h"

#include <vector>

#include <fbxsdk.h>

#include <fstream>
#include <filesystem>
#include <functional>

#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>

#include "../External/Imgui/imgui.h"

#include "../Inc/GraphicsCommon.h"
#include "../Inc/GraphicsDevice.h"
#include "../Inc/GraphicsCommandList.h"
#include "../Inc/CommandQueue.h"

#include "../Inc/ShaderCompiler.h"
#include "../../Assets/Shader/Common.hlsli"

#include "../Inc/FbxLoader.h"


#define _USE_CUBE_	0


namespace argent::graphics
{
	void Raytracer::Awake(const GraphicsDevice& graphics_device, GraphicsCommandList& command_list,
		CommandQueue& command_queue, UINT64 width, UINT height, 
			DescriptorHeap& cbv_srv_uav_descriptor_heap)
	{
		skymaps_[0] = std::make_unique<Texture>(&graphics_device, &command_queue, &cbv_srv_uav_descriptor_heap,
			L"./Assets/Images/Skymap00.dds");
		skymaps_[1] = std::make_unique<Texture>(&graphics_device, &command_queue, &cbv_srv_uav_descriptor_heap,
			L"./Assets/Images/Skymap01.dds");
		skymaps_[2] = std::make_unique<Texture>(&graphics_device, &command_queue, &cbv_srv_uav_descriptor_heap,
			L"./Assets/Images/Skymap02.dds");
		skymaps_[3] = std::make_unique<Texture>(&graphics_device, &command_queue, &cbv_srv_uav_descriptor_heap,
			L"./Assets/Images/Skymap03.dds");

		width_ = width;
		height_ = height;

		object_descriptor_ = cbv_srv_uav_descriptor_heap.PopDescriptor();

		transforms_[Plane].position_.y = -5.0f;
		transforms_[Plane].scaling_ = DirectX::XMFLOAT3(200.0f, 1.0f, 200.0f);
		transforms_[Coral0].position_.x = 3.0f;
		transforms_[SphereAABB].position_.z = 10.0f;

		//Initialize Sphere
		{
			materials_[Plane].albedo_color_ = float4(1.0f, 1.0f, 1.0f, 1.0f);
			materials_[Plane].diffuse_coefficient_ = 0.2f;
			materials_[Plane].specular_coefficient_ = 0.6f;
			materials_[Plane].specular_power_ = 50.f;
			materials_[Plane].reflectance_coefficient_ = 0.8f;
			materials_[SphereAABB].albedo_color_ = float4(1.0f, 1.0f, 1.0f, 1.0f);
			materials_[SphereAABB].diffuse_coefficient_ = 0.2f;
			materials_[SphereAABB].specular_coefficient_ = 0.9f;
			materials_[SphereAABB].specular_power_ = 50.f;
			materials_[SphereAABB].reflectance_coefficient_ = 1.0f;
		}


		//Fbx Loader
		//FbxLoader("./Assets/Model/Coral.fbx");

		for(int i = 0; i < GeometryTypeCount - kNoModelGeometryCounts; ++i)
		{
			model_[i] = game_resource::LoadFbx(filepaths[i].c_str());
		}

		for(auto& m : model_)
		{
			m->Awake(&graphics_device, &command_queue, &cbv_srv_uav_descriptor_heap);
		}

		CreateAS(graphics_device, command_list, command_queue);
		CreatePipeline(graphics_device);
		CreateOutputBuffer(graphics_device, width, height);
		CreateShaderResourceHeap(graphics_device, cbv_srv_uav_descriptor_heap);
		CreateShaderBindingTable(graphics_device);
	}

	void Raytracer::Shutdown()
	{
		for(auto& m : model_)
		{
			std::filesystem::path path = m->GetFilePath().c_str();
			path.replace_extension("cereal");

			std::ofstream ofs(path.c_str(), std::ios::binary);
			cereal::BinaryOutputArchive serialization(ofs);
			serialization(m);
		}
	}

	void Raytracer::Update(GraphicsCommandList* graphics_command_list, CommandQueue* upload_command_queue)
	{
		if(!is_wait_)
		{
			for(int i = 0; i < kSkymapCounts; ++i)
			{
				skymaps_[i]->WaitBeforeUse();
			}

			for(int i = 0; i < GeometryTypeCount - kNoModelGeometryCounts; ++i)
			{
				model_[i]->WaitBeforeUse();
			}
			is_wait_ = true;
		}
		for(auto& m : model_)
		{
			m->UpdateMaterialData();
		}
		if(ImGui::TreeNode("Skymap Texture"))
		{
			ImGui::SliderInt("Index", &skymap_index_, 0, kSkymapCounts - 1);
			for(int i = 0; i < kSkymapCounts; ++i)
			{
				ImGui::Image(reinterpret_cast<ImTextureID>(skymaps_[i]->GetGpuHandle().ptr), ImVec2(256, 256));
			}
			ImGui::TreePop();
		}

		memcpy(map_skymap_index_, &skymap_index_, sizeof(int));

		for(int i = 0; i < GeometryTypeCount; ++i)
		{
			if(ImGui::TreeNode(name[i].c_str()))
			{
				transforms_[i].OnGui();
				if(i < kNoModelGeometryCounts)
				{
					materials_[i].OnGui();
				}
				else
				{
					model_[i - kNoModelGeometryCounts]->OnGui();
				}
				ImGui::TreePop();
			}
		}

		graphics_command_list->Activate();

		for(int i = 0; i < GeometryTypeCount; ++i)
		{
			DirectX::XMMATRIX m = transforms_[i].CalcWorldMatrix();

			ObjectConstant obj_constant{};
			DirectX::XMStoreFloat4x4(&obj_constant.world_, m);
			DirectX::XMStoreFloat4x4(&obj_constant.inv_world_, DirectX::XMMatrixInverse(nullptr, m));
			memcpy(world_mat_map_ + i * sizeof(ObjectConstant), &obj_constant, sizeof(ObjectConstant));
			if(i < kNoModelGeometryCounts)
				memcpy(material_map_ + i * sizeof(Material), &materials_[i], sizeof(Material));

			as_manager_.SetWorld(obj_constant.world_, tlas_unique_id_[i]);
		}
		as_manager_.Update(graphics_command_list);

		graphics_command_list->Deactivate();
		ID3D12CommandList* command_lists[]{ graphics_command_list->GetCommandList() };
		upload_command_queue->Execute(1u, command_lists);
		upload_command_queue->Signal();
		upload_command_queue->WaitForGpu();
	}

	void Raytracer::OnRender(const GraphicsCommandList& graphics_command_list, D3D12_GPU_VIRTUAL_ADDRESS scene_constant_gpu_handle)
	{
		auto command_list = graphics_command_list.GetCommandList4();

		graphics_command_list.SetTransitionBarrier(output_buffer_.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		D3D12_DISPATCH_RAYS_DESC desc{};
		desc.RayGenerationShaderRecord.StartAddress = raygen_shader_table_.GetGpuVirtualAddress();
		desc.RayGenerationShaderRecord.SizeInBytes = raygen_shader_table_.GetSize();
		desc.MissShaderTable.StartAddress = miss_shader_table_.GetGpuVirtualAddress();
		desc.MissShaderTable.SizeInBytes = miss_shader_table_.GetSize();
		desc.MissShaderTable.StrideInBytes = miss_shader_table_.GetStride();
		desc.HitGroupTable.StartAddress = hit_group_shader_table_.GetGpuVirtualAddress();
		desc.HitGroupTable.SizeInBytes = hit_group_shader_table_.GetSize();
		desc.HitGroupTable.StrideInBytes = hit_group_shader_table_.GetStride();	
		desc.Width = static_cast<UINT>(width_);
		desc.Height = height_;
		desc.Depth = 1;

		//Set Global Raytracing RootSignature Resource
		command_list->SetComputeRootSignature(global_root_signature_.GetRootSignatureObject());
		//OutputBuffer & TLAS
		command_list->SetComputeRootDescriptorTable(0u, output_descriptor_.gpu_handle_);
		command_list->SetComputeRootConstantBufferView(1u, scene_constant_gpu_handle);

		command_list->SetPipelineState1(pipeline_state_.GetStateObject());
		command_list->DispatchRays(&desc);

		graphics_command_list.SetTransitionBarrier(output_buffer_.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	}

	void Raytracer::BuildGeometry(const GraphicsDevice& graphics_device)
	{
		//Plane
		{
			Vertex vertices1[6]
			{
				{{ -1.0f, 0.0f, 1.0f}, {}},
				{{ 1.0f, 0.0f, 1.0}, {}},
				{{ -1.0f, 0.0f, -1.0f}, {}},

				{{ -1.0f, 0.0f, -1.0f}, {}},
				{{ 1.0f, 0.0f, 1.0f}, {}},
				{{ 1.0f, 0.0f, -1.0f}, {}},
			};

			vertex_buffers_[Plane] = std::make_unique<VertexBuffer>(&graphics_device, vertices1, sizeof(Vertex), 6u);
		}

		//AABB
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> aabb_buffer_;
			D3D12_RAYTRACING_AABB rt_aabb;
			float aabb_size = 10.0f;

			rt_aabb.MaxX = 
			rt_aabb.MaxY = 
			rt_aabb.MaxZ = aabb_size;
			rt_aabb.MinX = 
			rt_aabb.MinY = 
			rt_aabb.MinZ = -aabb_size;

			vertex_buffers_[SphereAABB] = std::make_unique<VertexBuffer>(&graphics_device,
				&rt_aabb, sizeof(D3D12_RAYTRACING_AABB), 1u);
		}
	}

	void Raytracer::CreateAS(const GraphicsDevice& graphics_device, 
	                         GraphicsCommandList& command_list, CommandQueue& command_queue)
	{
		BuildGeometry(graphics_device);

		uint unique_id[GeometryTypeCount];

		//Add Bottom Level
		for(int i = 0; i < GeometryTypeCount; ++i)
		{
			bool triangle = i != SphereAABB;
			dxr::BLASBuildDesc build_desc;

			if(i < kNoModelGeometryCounts)
			{
				build_desc.vertex_buffer_vec_.emplace_back(vertex_buffers_[i].get());
			}
			else
			{
				build_desc.vertex_buffer_vec_.emplace_back(model_[i - kNoModelGeometryCounts]->GetMesh()->GetVertexBuffer());				
				build_desc.index_buffer_vec_.emplace_back(model_[i - kNoModelGeometryCounts]->GetMesh()->GetIndexBuffer());				
			}
			unique_id[i] = as_manager_.AddBottomLevelAS(&graphics_device, &command_list, &build_desc, triangle);
		}

		for(int i = 0; i < GeometryTypeCount; ++i)
		{
			DirectX::XMFLOAT4X4 m;
			DirectX::XMStoreFloat4x4(&m, transforms_[i].CalcWorldMatrix());
			bool front_counter_clockwise = false;
			tlas_unique_id_[i] = as_manager_.RegisterTopLevelAS(unique_id[i], i, m, front_counter_clockwise);
		}

		as_manager_.Generate(&graphics_device, &command_list);

		//Execute Command list
		command_list.Deactivate();

		ID3D12CommandList* command_lists[]
		{
			command_list.GetCommandList(),
		};
		command_queue.Execute(1u, command_lists);
		command_queue.Signal();
		command_queue.WaitForGpu();
	}

	void Raytracer::CreatePipeline(const GraphicsDevice& graphics_device)
	{
		//Global Root Signature
		global_root_signature_.AddHeapRangeParameters(
			{
			{0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_UAV},
			{0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV}
			}
		);
		global_root_signature_.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, 0u, 0u, 1u);
		global_root_signature_.Create(&graphics_device, false);

		//Shared root signature
		raygen_miss_root_signature_.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, 0u, 1u, 1u);
		raygen_miss_root_signature_.AddHeapRangeParameter(0u, kSkymapCounts, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1u);
		raygen_miss_root_signature_.Create(&graphics_device, true);

		//Hit Group Root Signature
		hit_group_root_signature_.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, 0u, 1u, 1u);	//For Instance ID
		hit_group_root_signature_.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, 1u, 1u, 1u);	//For Material Constant
		hit_group_root_signature_.AddHeapRangeParameter(0u, 1u, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1u);	//For Abledo
		hit_group_root_signature_.AddHeapRangeParameter(1u, 1u, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1u);	//For Normal
		hit_group_root_signature_.AddHeapRangeParameter(2u, 2u, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1u);	//For Vertex Buffer and Index Buffer
		hit_group_root_signature_.Create(&graphics_device, true);

		//Compile the shader library.
		ShaderCompiler shader_compiler;
		shader_compiler.CompileShaderLibrary(L"./Assets/Shader/RayGen.hlsl", ray_gen_library_.ReleaseAndGetAddressOf());;
		shader_compiler.CompileShaderLibrary(L"./Assets/Shader/Miss.hlsl", miss_library_.ReleaseAndGetAddressOf());
		shader_compiler.CompileShaderLibrary(L"./Assets/Shader/Plane.lib.hlsl", plane_library_.ReleaseAndGetAddressOf());
		shader_compiler.CompileShaderLibrary(L"./Assets/Shader/StaticMesh.lib.hlsl", static_mesh_library_.ReleaseAndGetAddressOf());
		shader_compiler.CompileShaderLibrary(L"./Assets/Shader/Sphere.lib.hlsl", sphere_library_.ReleaseAndGetAddressOf());

		//Add Shader Library
		{
			pipeline_state_.AddLibrary(ray_gen_library_.Get(), {L"RayGen"});
			pipeline_state_.AddLibrary(miss_library_.Get(), {L"Miss"});
			pipeline_state_.AddLibrary(plane_library_.Get(), {L"PlaneClosestHit"});
			pipeline_state_.AddLibrary(static_mesh_library_.Get(), {L"StaticMeshClosestHit"});
			pipeline_state_.AddLibrary(sphere_library_.Get(), {{L"SphereClosestHit"}, {L"SphereIntersection"}});
		}

		//Add Hit Group
		{
			pipeline_state_.AddHitGroup(kHitGroupName[Plane], L"PlaneClosestHit");
			pipeline_state_.AddHitGroup(kHitGroupName[SphereAABB], L"SphereClosestHit", L"", L"SphereIntersection");

			for(int i = kNoModelGeometryCounts; i < GeometryTypeCount; ++i)
			{
				pipeline_state_.AddHitGroup(kHitGroupName[i],  L"StaticMeshClosestHit");
			}
			//pipeline_state_.AddHitGroup(kHitGroupName[Coral0], L"StaticMeshClosestHit");
			//pipeline_state_.AddHitGroup(kHitGroupName[Coral1], L"StaticMeshClosestHit");
		}

		//Add Root Signature Association
		{
			pipeline_state_.AddRootSignatureAssociation(raygen_miss_root_signature_.GetRootSignatureObject(), {{L"RayGen"}, {L"Miss"}});
			pipeline_state_.AddRootSignatureAssociation(hit_group_root_signature_.GetRootSignatureObject(), kHitGroupName);
		}

		pipeline_state_.SetMaxAttributeSize(3 * sizeof(float));
		pipeline_state_.SetMaxPayloadSize(sizeof(RayPayload));
		pipeline_state_.SetMaxRecursionDepth(_MAX_RECURSION_DEPTH_);
		pipeline_state_.Generate(&graphics_device, global_root_signature_.GetRootSignatureObject(), raygen_miss_root_signature_.GetRootSignatureObject());
	}

	void Raytracer::CreateOutputBuffer(const GraphicsDevice& graphics_device, UINT64 width, UINT height)
	{
		graphics_device.CreateTexture2D(kDefaultHeapProp, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			DXGI_FORMAT_R8G8B8A8_UNORM, width, height, D3D12_RESOURCE_STATE_COPY_SOURCE, 
			output_buffer_.ReleaseAndGetAddressOf());
	}

	void Raytracer::CreateShaderResourceHeap(const GraphicsDevice& graphics_device, 
			DescriptorHeap& cbv_srv_uav_descriptor_heap)
	{
		output_descriptor_ = cbv_srv_uav_descriptor_heap.PopDescriptor();
		tlas_result_descriptor_ = cbv_srv_uav_descriptor_heap.PopDescriptor();

		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
		uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		graphics_device.GetDevice()->CreateUnorderedAccessView(output_buffer_.Get(), 
			nullptr, &uav_desc, output_descriptor_.cpu_handle_);

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
		srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.RaytracingAccelerationStructure.Location = as_manager_.GetResultResourceObject()->GetGPUVirtualAddress();

		graphics_device.CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, 
			sizeof(Material) * as_manager_.GetInstanceCounts(), 
			D3D12_RESOURCE_STATE_GENERIC_READ, material_buffer_.ReleaseAndGetAddressOf());

		graphics_device.GetDevice()->CreateShaderResourceView(nullptr, &srv_desc,
			tlas_result_descriptor_.cpu_handle_);

		material_buffer_->Map(0u, nullptr, reinterpret_cast<void**>(&material_map_));
		for(int i = 0; i < GeometryTypeCount; ++i)
		{
			memcpy(material_map_ + i * sizeof(Material), &materials_[i], sizeof(Material));
		}

		//All Instance World Matrix Buffer
		uint stride = sizeof(ObjectConstant);
		uint num = as_manager_.GetInstanceCounts();

		graphics_device.CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE,
			stride * num,
			D3D12_RESOURCE_STATE_GENERIC_READ, world_matrix_buffer_.ReleaseAndGetAddressOf());

		world_matrix_buffer_->Map(0u, nullptr, reinterpret_cast<void**>(&world_mat_map_));

		for(int i = 0; i < GeometryTypeCount; ++i)
		{
			auto m = transforms_[i].CalcWorldMatrix();
			ObjectConstant data;
			DirectX::XMStoreFloat4x4(&data.world_, m);
			DirectX::XMStoreFloat4x4(&data.inv_world_, DirectX::XMMatrixInverse(nullptr, m));
			//memcpy_s(map, stride * num, &f4x4, stride);
			memcpy(world_mat_map_, &data, stride);
		}

		world_matrix_buffer_->Unmap(0u, nullptr);

		//Skymap buffer
		graphics_device.CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE,
			sizeof(int), D3D12_RESOURCE_STATE_GENERIC_READ, skymap_index_buffer_.ReleaseAndGetAddressOf());

		skymap_index_buffer_->Map(0u, nullptr, reinterpret_cast<void**>(&map_skymap_index_));
	}

	void Raytracer::CreateShaderBindingTable(const GraphicsDevice& graphics_device)
	{

		{
			raygen_shader_table_.AddShaderIdentifier(L"RayGen");
			raygen_shader_table_.Generate(&graphics_device, 
				pipeline_state_.GetStateObjectProperties(), L"RayGenerationShaderTable");
		}

		//Miss Shader
		{
			std::vector<void*> data(kSkymapCounts + 1);
			data.at(0u) = reinterpret_cast<void*>(skymap_index_buffer_->GetGPUVirtualAddress());
			for (int i = 0; i < kSkymapCounts; ++i)
			{
				data.at(i + 1) = reinterpret_cast<void*>(skymaps_[i]->GetGpuHandle().ptr);
			}
			miss_shader_table_.AddShaderIdentifierAndInputData(L"Miss", data);
			miss_shader_table_.Generate(&graphics_device, 
				pipeline_state_.GetStateObjectProperties(), L"MissShaderTable");
		}

		//hit
		{
			std::vector<dxr::ShaderTable> tables(GeometryTypeCount);

			//For Plane
			tables.at(Plane).shader_identifier_ = kHitGroupName[Plane];
			tables.at(Plane).input_data_.resize(RootSignatureBinderCount);
			tables.at(Plane).input_data_.at(ObjectCbv) = reinterpret_cast<void*>(world_matrix_buffer_->GetGPUVirtualAddress() + sizeof(ObjectConstant) * Plane);
			tables.at(Plane).input_data_.at(MaterialCbv) = reinterpret_cast<void*>(material_buffer_->GetGPUVirtualAddress() + sizeof(Material) * Plane);
			tables.at(Plane).input_data_.at(VertexBufferGpuDescriptorHandle) = reinterpret_cast<void*>(0);

			//For Sphere
			tables.at(SphereAABB).shader_identifier_ = kHitGroupName.at(SphereAABB);
			tables.at(SphereAABB).input_data_.resize(RootSignatureBinderCount);
			tables.at(SphereAABB).input_data_.at(MaterialCbv) = reinterpret_cast<void*>(material_buffer_->GetGPUVirtualAddress() + sizeof(Material) * SphereAABB);
			tables.at(SphereAABB).input_data_.at(ObjectCbv) = reinterpret_cast<void*>(world_matrix_buffer_->GetGPUVirtualAddress() + sizeof(ObjectConstant) * SphereAABB);

			//For Coral

			for(size_t i = 0; i < GeometryTypeCount - kNoModelGeometryCounts; ++i)
			{
				auto& table = tables.at(kNoModelGeometryCounts + i);
				table.shader_identifier_ = kHitGroupName.at(kNoModelGeometryCounts + i);
				table.input_data_.resize(RootSignatureBinderCount);
				table.input_data_.at(ObjectCbv) = reinterpret_cast<void*>(world_matrix_buffer_->GetGPUVirtualAddress() + sizeof(ObjectConstant) * (kNoModelGeometryCounts + i));
				auto model_data = model_[i]->GetShaderBindingData();
				for(size_t j = 0; j < model_data.size(); ++j)
				{
					table.input_data_.at(j + 1) = model_data.at(j);
				}
			}

			hit_group_shader_table_.AddShaderTables(tables);
			hit_group_shader_table_.Generate(&graphics_device, 
				pipeline_state_.GetStateObjectProperties(), L"HitGroupShaderTable");
		}
	}

	//For Fbx
	struct Scene
	{
		struct Node
		{
			uint64_t unique_id_{};
			std::string name_;
			UINT attribute_{};
			int64_t parent_index_{ -1 };
		};

		std::vector<Node> nodes_;
		int64_t IndexOf(uint64_t unique_id) const
		{
			int64_t index{ 0 };
			for (const auto& node : nodes_)
			{
				if (node.unique_id_ == unique_id)
					return index;
				++index;
			}
			return -1;
		}
	};
	Scene scene_view_;
	void FetchMesh(FbxScene* fbx_scene, std::vector<Raytracer::Mesh>& meshes);
	
	void Raytracer::FbxLoader(const char* filename)
	{
		FbxManager* fbx_manager{ FbxManager::Create() };
		FbxScene* fbx_scene{ FbxScene::Create(fbx_manager, "") };

		FbxImporter* fbx_importer{ FbxImporter::Create(fbx_manager, "") };
		bool import_status{ false };
		import_status = fbx_importer->Initialize(filename);
		_ASSERT_EXPR(import_status, L"Failed to Import Model");

		import_status = fbx_importer->Import(fbx_scene);
		_ASSERT_EXPR(import_status, L"Failed to Import Model");

		FbxGeometryConverter fbx_converter(fbx_manager);

		std::function<void(FbxNode*)> Traverse{[&](FbxNode* fbx_node)
		{
			Scene::Node& node{ scene_view_.nodes_.emplace_back() };
			node.attribute_ = fbx_node->GetNodeAttribute() ?
				fbx_node->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown;
			node.name_ = fbx_node->GetName();
			node.unique_id_ = fbx_node->GetUniqueID();
			node.parent_index_ = scene_view_.IndexOf(fbx_node->GetParent() ? 
				fbx_node->GetParent()->GetUniqueID() : 0);
			for(int child_index = 0; child_index < fbx_node->GetChildCount(); ++child_index)
			{
				Traverse(fbx_node->GetChild(child_index));
			}
		}};
		Traverse(fbx_scene->GetRootNode());

		
		FetchMesh(fbx_scene, meshes_);

		fbx_manager->Destroy();
	}

	void FetchMesh(FbxScene* fbx_scene, std::vector<Raytracer::Mesh>& meshes)
	//Fbxシーンからメッシュのアトリビュートを持っているノードを探し、そこから
	//メッシュデータを取って来る
	{
		for(const auto& node : scene_view_.nodes_)
		{
			if (node.attribute_ != FbxNodeAttribute::EType::eMesh) continue;

			//TODO FindNodeByNameは欠陥品なので修正する
			auto* fbx_node{ fbx_scene->FindNodeByName(node.name_.c_str()) };
			auto* fbx_mesh{ fbx_node->GetMesh() };

			auto& mesh{ meshes.emplace_back() };
			mesh.unique_id_ = fbx_mesh->GetNode()->GetUniqueID();
			mesh.name_ = fbx_mesh->GetNode()->GetName();
			mesh.node_index_ = scene_view_.IndexOf(mesh.unique_id_);

			const int polygon_count{ fbx_mesh->GetPolygonCount() };
			mesh.vertices_.resize(polygon_count * 3LL);
			mesh.indices_.resize(polygon_count * 3LL);

			FbxStringList uv_names;
			fbx_mesh->GetUVSetNames(uv_names);
			const FbxVector4* control_points{ fbx_mesh->GetControlPoints() };
			for (int polygon_index = 0; polygon_index < polygon_count; ++polygon_index)
			{
				for (int position_in_polygon = 0; position_in_polygon < 3; ++position_in_polygon)
				{
					const int vertex_index{ polygon_index * 3 + position_in_polygon };

					Vertex vertex;
					const int polygon_vertex{ fbx_mesh->GetPolygonVertex(polygon_index, position_in_polygon) };
					vertex.position_.x = static_cast<float>(control_points[polygon_vertex][0]);
					vertex.position_.y = static_cast<float>(control_points[polygon_vertex][1]);
					vertex.position_.z = static_cast<float>(control_points[polygon_vertex][2]);

					if (fbx_mesh->GetElementNormalCount() > 0)
					{
						FbxVector4 normal;
						fbx_mesh->GetPolygonVertexNormal(polygon_index, position_in_polygon, normal);
						vertex.normal_.x = static_cast<float>(normal[0]);
						vertex.normal_.y = static_cast<float>(normal[1]);
						vertex.normal_.z = static_cast<float>(normal[2]);
					}

					if (fbx_mesh->GetElementUVCount() > 0)
					{
						FbxVector2 uv;
						bool unmapped_uv;
						fbx_mesh->GetPolygonVertexUV(polygon_index, position_in_polygon, uv_names[0], uv, unmapped_uv);
						vertex.texcoord_.x = static_cast<float>(uv[0]);
						vertex.texcoord_.y = static_cast<float>(uv[1]);
					}

					if(fbx_mesh->GenerateTangentsData(0, false))
					{
						const FbxGeometryElementTangent* tangent = fbx_mesh->GetElementTangent(0);
						vertex.tangent_.x = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[0]);
						vertex.tangent_.y = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[1]);
						vertex.tangent_.z = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[2]);
				//		vertex.tangent_.w = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[3]);
					}

					if(fbx_mesh->GetElementBinormalCount() <= 0)
					{
						fbx_mesh->CreateElementBinormal();
					}

					const FbxGeometryElementBinormal* binormal = fbx_mesh->GetElementBinormal(0u);
					vertex.binormal_.x = static_cast<float>(binormal->GetDirectArray().GetAt(vertex_index)[0]);
					vertex.binormal_.y = static_cast<float>(binormal->GetDirectArray().GetAt(vertex_index)[1]);
					vertex.binormal_.z = static_cast<float>(binormal->GetDirectArray().GetAt(vertex_index)[2]);
					//vertex.binormal_.w = static_cast<float>(binormal->GetDirectArray().GetAt(vertex_index)[3]);
					

					mesh.vertices_.at(vertex_index) = std::move(vertex);
					mesh.indices_.at(vertex_index) = vertex_index;
				}
			}
		}
	}
}
