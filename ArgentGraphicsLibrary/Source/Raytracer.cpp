#include "../Inc/Raytracer.h"

#include <vector>

#include <fbxsdk.h>

#include <fstream>
#include <filesystem>
#include <functional>

#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>

#include "../External/Imgui/imgui.h"

#include "../Inc/D3D12Common.h"
#include "../Inc/GraphicsCommon.h"
#include "../Inc/GraphicsDevice.h"
#include "../Inc/GraphicsCommandList.h"
#include "../Inc/CommandQueue.h"

#include "../Inc/ShaderCompiler.h"
#include "../../Assets/Shader/Common.hlsli"

#include "../Inc/FbxLoader.h"


namespace argent::graphics
{
	void Raytracer::Awake(const dx12::GraphicsDevice& graphics_device, dx12::GraphicsCommandList& command_list,
		dx12::CommandQueue& command_queue, UINT64 width, UINT height, 
			dx12::DescriptorHeap& cbv_srv_uav_descriptor_heap)
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

		transforms_[Plane].position_ = {0.0f, 0.0f, 0.0f };
		transforms_[Plane].scaling_ = DirectX::XMFLOAT3(1000.0f, 1.0f, 1000.0f);
		transforms_[Sphere].position_ = {0.0f, 50.0f, 0.0f };
		//For Coral Group
		{
			transforms_[CoralRock].position_ = { -100.0f, 0.0f, 0.0f };
			transforms_[CoralRock].scaling_ = { 0.1f, 0.1f, 0.1f };
			transforms_[CoralRock].rotation_ = { -1.57f, 0.0f, 0.0f };
			transforms_[Coral8].position_ = { 100.0f, 0.0f, 0.0f};
			transforms_[Coral8].rotation_ = { -1.57f, -0.0f, 0.0f};
		}
		//Initialize Sphere
		{
			materials_[Plane].albedo_color_ = float4(1.0f, 1.0f, 1.0f, 1.0f);
			materials_[Plane].diffuse_coefficient_ = 0.2f;
			materials_[Plane].specular_coefficient_ = 0.6f;
			materials_[Plane].specular_power_ = 50.f;
			materials_[Plane].reflectance_coefficient_ = 0.8f;
			materials_[Sphere].albedo_color_ = float4(1.0f, 1.0f, 1.0f, 1.0f);
			materials_[Sphere].diffuse_coefficient_ = 0.2f;
			materials_[Sphere].specular_coefficient_ = 0.9f;
			materials_[Sphere].specular_power_ = 50.f;
			materials_[Sphere].reflectance_coefficient_ = 1.0f;
		}


		//Fbx Loader
		//FbxLoader("./Assets/Model/Coral.fbx");

		for(int i = 0; i < GeometryTypeCount - kNoModelGeometryCounts; ++i)
		{
			model_[i] = game_resource::LoadFbx(filepaths[i].c_str());
			vertex_counts_ += model_[i]->GetMesh()->GetVertexCounts(); 
			index_counts_ += model_[i]->GetMesh()->GetIndexCounts(); 
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

	void Raytracer::Update(dx12::GraphicsCommandList* graphics_command_list, dx12::CommandQueue* upload_command_queue)
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

	void Raytracer::OnRender(const dx12::GraphicsCommandList& graphics_command_list, D3D12_GPU_VIRTUAL_ADDRESS scene_constant_gpu_handle)
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

	void Raytracer::BuildGeometry(const dx12::GraphicsDevice& graphics_device)
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

			vertex_buffers_[Plane] = std::make_unique<dx12::VertexBuffer>(&graphics_device, vertices1, sizeof(Vertex), 6u);
		}

		//AABB
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> aabb_buffer_;
			D3D12_RAYTRACING_AABB rt_aabb;
			float aabb_size = 500.0f;

			rt_aabb.MaxX = 
			rt_aabb.MaxY = 
			rt_aabb.MaxZ = aabb_size;
			rt_aabb.MinX = 
			rt_aabb.MinY = 
			rt_aabb.MinZ = -aabb_size;

			vertex_buffers_[Sphere] = std::make_unique<dx12::VertexBuffer>(&graphics_device,
			                                                               &rt_aabb, sizeof(D3D12_RAYTRACING_AABB), 1u);


			aabb_size = 1000.0f;

			rt_aabb.MaxX = 
			rt_aabb.MaxY = 
			rt_aabb.MaxZ = aabb_size;
			rt_aabb.MinX = 
			rt_aabb.MinY = 
			rt_aabb.MinZ = -aabb_size;

			//vertex_buffers_[Sphere1] = std::make_unique<VertexBuffer>(&graphics_device,
			//	&rt_aabb, sizeof(D3D12_RAYTRACING_AABB), 1u);

		}
	}

	void Raytracer::CreateAS(const dx12::GraphicsDevice& graphics_device,
	                         dx12::GraphicsCommandList& command_list, dx12::CommandQueue& command_queue)
	{
		BuildGeometry(graphics_device);

		uint unique_id[GeometryTypeCount];

		//Add Bottom Level
		for(int i = 0; i < GeometryTypeCount; ++i)
		{
			bool triangle = i != Sphere;
			dx12::BLASBuildDesc build_desc;

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

	void Raytracer::CreatePipeline(const dx12::GraphicsDevice& graphics_device)
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
			pipeline_state_.AddHitGroup(kHitGroupName[Sphere], L"SphereClosestHit", L"", L"SphereIntersection");

			for(int i = kNoModelGeometryCounts; i < GeometryTypeCount; ++i)
			{
				pipeline_state_.AddHitGroup(kHitGroupName[i],  L"StaticMeshClosestHit");
			}
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

	void Raytracer::CreateOutputBuffer(const dx12::GraphicsDevice& graphics_device, UINT64 width, UINT height)
	{
		graphics_device.CreateTexture2D(dx12::kDefaultHeapProp, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			DXGI_FORMAT_R8G8B8A8_UNORM, static_cast<UINT>(width), height, D3D12_RESOURCE_STATE_COPY_SOURCE, 
			output_buffer_.ReleaseAndGetAddressOf());
	}

	void Raytracer::CreateShaderResourceHeap(const dx12::GraphicsDevice& graphics_device,
	                                         dx12::DescriptorHeap& cbv_srv_uav_descriptor_heap)
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

		graphics_device.CreateBuffer(dx12::kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, 
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

		graphics_device.CreateBuffer(dx12::kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE,
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
		graphics_device.CreateBuffer(dx12::kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE,
			sizeof(int), D3D12_RESOURCE_STATE_GENERIC_READ, skymap_index_buffer_.ReleaseAndGetAddressOf());

		skymap_index_buffer_->Map(0u, nullptr, reinterpret_cast<void**>(&map_skymap_index_));
	}

	void Raytracer::CreateShaderBindingTable(const dx12::GraphicsDevice& graphics_device)
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
			std::vector<dx12::ShaderTable> tables(GeometryTypeCount);

			//For Plane
			tables.at(Plane).shader_identifier_ = kHitGroupName[Plane];
			tables.at(Plane).input_data_.resize(RootSignatureBinderCount);
			tables.at(Plane).input_data_.at(ObjectCbv) = reinterpret_cast<void*>(world_matrix_buffer_->GetGPUVirtualAddress() + sizeof(ObjectConstant) * Plane);
			tables.at(Plane).input_data_.at(MaterialCbv) = reinterpret_cast<void*>(material_buffer_->GetGPUVirtualAddress() + sizeof(Material) * Plane);
			tables.at(Plane).input_data_.at(VertexBufferGpuDescriptorHandle) = reinterpret_cast<void*>(0);

			//For Sphere
			tables.at(Sphere).shader_identifier_ = kHitGroupName.at(Sphere);
			tables.at(Sphere).input_data_.resize(RootSignatureBinderCount);
			tables.at(Sphere).input_data_.at(MaterialCbv) = reinterpret_cast<void*>(material_buffer_->GetGPUVirtualAddress() + sizeof(Material) * Sphere);
			tables.at(Sphere).input_data_.at(ObjectCbv) = reinterpret_cast<void*>(world_matrix_buffer_->GetGPUVirtualAddress() + sizeof(ObjectConstant) * Sphere);


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
}
