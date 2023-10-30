#include "Graphics/Raytracer.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <vector>

#include <imgui.h>

#include "Graphics/API/D3D12/D3D12Common.h"
#include "Graphics/API/D3D12/GraphicsDevice.h"
#include "Graphics/API/D3D12/CommandQueue.h"
#include "Graphics/API/D3D12/GraphicsCommandList.h"

#include "Graphics/Common/GraphicsCommon.h"
#include "Graphics/Common/GraphicsContext.h"

#include "Graphics/Wrapper/ShaderCompiler.h"

#include "Graphics/GameResource/Mesh.h"
#include "Graphics/GameResource/Material.h"
#include "Graphics/GameResource/Model.h"

#include "Graphics/Loader/FbxLoader.h"

#include "../../Assets/Shader/Common.hlsli"

namespace argent::graphics
{
	void Raytracer::Awake(const dx12::GraphicsDevice& graphics_device, dx12::GraphicsCommandList& command_list,
		dx12::CommandQueue& command_queue, UINT64 width, UINT height, dx12::DescriptorHeap& cbv_srv_uav_descriptor_heap,
		const GraphicsContext* graphics_context)
	{
		skymaps_ = std::make_unique<Texture>(&graphics_device, &command_queue, &cbv_srv_uav_descriptor_heap,
			L"./Assets/Images/Skymap00.dds");


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
		}

		//TODO �V�����o�[�W������Loader����
		graphics_model_ = LoadFbxFromFile("./Assets/Model/Plantune.FBX");
	
		graphics_model_->Awake(graphics_context);

		CreateAS(graphics_device, command_list, command_queue);
		CreatePipeline(graphics_device);
		CreateOutputBuffer(graphics_device, width, height);
		CreateShaderResourceHeap(graphics_device, cbv_srv_uav_descriptor_heap);
		CreateShaderBindingTable(graphics_device);

	}

	void Raytracer::Shutdown()
	{
#if _USE_MODEL0_
		for(auto& m : model_)
		{
			std::filesystem::path path = m->GetFilePath().c_str();
			path.replace_extension("cereal");

			std::ofstream ofs(path.c_str(), std::ios::binary);
			cereal::BinaryOutputArchive serialization(ofs);
			serialization(m);
		}
#endif
	}

	void Raytracer::Update(dx12::GraphicsCommandList* graphics_command_list, dx12::CommandQueue* upload_command_queue)
	{
		if(!is_wait_)
		{
			skymaps_->WaitBeforeUse();

#if _USE_MODEL0_
			for(int i = 0; i < GeometryTypeCount - kNoModelGeometryCounts; ++i)
			{
				model_[i]->WaitBeforeUse();
			}
#else
			graphics_model_->WaitForUploadGpuResource();

#endif
			is_wait_ = true;
		}



#if _USE_MODEL0_
		for(auto& m : model_)
		{
			m->UpdateMaterialData();
		}
#else
		graphics_model_->GetMaterials().at(0)->UpdateConstantBuffer(0u);
#endif


		if(ImGui::TreeNode("Skymap Texture"))
		{
			ImGui::Image(reinterpret_cast<ImTextureID>(skymaps_->GetGpuHandle().ptr), ImVec2(256, 256));
			ImGui::TreePop();
		}

		for(int i = 0; i < GeometryTypeCount; ++i)
		{
			if(ImGui::TreeNode(name[i].c_str()))
			{
				transforms_[i].OnGui();
				ImGui::TreePop();
			}
		}

		graphics_model_->OnGui();

		graphics_command_list->Activate();

		for(int i = 0; i < GeometryTypeCount; ++i)
		{
			DirectX::XMMATRIX m = transforms_[i].CalcWorldMatrix();

			ObjectConstant obj_constant{};
			DirectX::XMStoreFloat4x4(&obj_constant.world_, m);
			DirectX::XMStoreFloat4x4(&obj_constant.inv_world_, DirectX::XMMatrixInverse(nullptr, m));
			memcpy(world_mat_map_ + i * sizeof(ObjectConstant), &obj_constant, sizeof(ObjectConstant));

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
#if _USE_MODEL0_
				build_desc.vertex_buffer_vec_.emplace_back(model_[i - kNoModelGeometryCounts]->GetMesh()->GetVertexBuffer());				
				build_desc.index_buffer_vec_.emplace_back(model_[i - kNoModelGeometryCounts]->GetMesh()->GetIndexBuffer());
#else
				build_desc.vertex_buffer_vec_.emplace_back(graphics_model_->GetMeshes().at(0)->GetPositionBuffer());				
				build_desc.index_buffer_vec_.emplace_back(graphics_model_->GetMeshes().at(0)->GetIndexBuffer());				

#endif
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
		raygen_miss_root_signature_.AddHeapRangeParameter(0u, 1u, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1u);
		raygen_miss_root_signature_.Create(&graphics_device, true);

		//Hit Group Root Signature
		hit_group_root_signature_.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, 0u, 1u, 1u);	//For Instance ID
		hit_group_root_signature_.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, 1u, 1u, 1u);	//For Material Constant
		hit_group_root_signature_.AddHeapRangeParameter(0u, 2u, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1u);	//For Abledo
#if _USE_MODEL0_
		hit_group_root_signature_.AddHeapRangeParameter(2u, 2u, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1u);	//For Vertex Buffer and Index Buffer
#else
		hit_group_root_signature_.AddHeapRangeParameter(2u, 6u, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1u);	//For Vertex Buffer and Index Buffer
#endif
		hit_group_root_signature_.Create(&graphics_device, true);

		//Compile the shader library.
		ShaderCompiler shader_compiler;
		shader_compiler.CompileShaderLibrary(L"./Assets/Shader/RayGen.hlsl", ray_gen_library_.ReleaseAndGetAddressOf());;
		shader_compiler.CompileShaderLibrary(L"./Assets/Shader/Miss.hlsl", miss_library_.ReleaseAndGetAddressOf());
		shader_compiler.CompileShaderLibrary(L"./Assets/Shader/Plane.lib.hlsl", plane_library_.ReleaseAndGetAddressOf());
#if _USE_MODEL0_
		shader_compiler.CompileShaderLibrary(L"./Assets/Shader/StaticMesh.lib.hlsl", static_mesh_library_.ReleaseAndGetAddressOf());
#else
		shader_compiler.CompileShaderLibrary(L"./Assets/Shader/StandardMesh.lib.hlsl", static_mesh_library_.ReleaseAndGetAddressOf());
#endif
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


		graphics_device.GetDevice()->CreateShaderResourceView(nullptr, &srv_desc,
			tlas_result_descriptor_.cpu_handle_);


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
			std::vector<void*> data(1 + 1);
			data.at(1) = reinterpret_cast<void*>(skymaps_->GetGpuHandle().ptr);
			
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
			tables.at(Plane).input_data_.at(VertexBufferGpuDescriptorHandle) = reinterpret_cast<void*>(0);

			//For Sphere
			tables.at(Sphere).shader_identifier_ = kHitGroupName.at(Sphere);
			tables.at(Sphere).input_data_.resize(RootSignatureBinderCount);
			tables.at(Sphere).input_data_.at(ObjectCbv) = reinterpret_cast<void*>(world_matrix_buffer_->GetGPUVirtualAddress() + sizeof(ObjectConstant) * Sphere);


			for(size_t i = 0; i < GeometryTypeCount - kNoModelGeometryCounts; ++i)
			{
				auto& table = tables.at(kNoModelGeometryCounts + i);
				table.shader_identifier_ = kHitGroupName.at(kNoModelGeometryCounts + i);
				table.input_data_.resize(RootSignatureBinderCount);
				table.input_data_.at(ObjectCbv) = reinterpret_cast<void*>(world_matrix_buffer_->GetGPUVirtualAddress() + sizeof(ObjectConstant) * (kNoModelGeometryCounts + i));
				table.input_data_.at(TextureStart) = reinterpret_cast<void*>(graphics_model_->GetMaterials().at(0)->GetTextureGpuHandleBegin().ptr);
				table.input_data_.at(VertexBufferGpuDescriptorHandle) = reinterpret_cast<void*>(
					graphics_model_->GetMeshes().at(0)->GetVertexPositionDescriptor().gpu_handle_.ptr);
				table.input_data_.at(MaterialCbv) = reinterpret_cast<void*>(graphics_model_->GetMaterials().at(0)->GetConstantGpuVirtualAddress(0));
				//auto model_data = model_[i]->GetShaderBindingData();
				//for(size_t j = 0; j < model_data.size(); ++j)
				//{
				//	table.input_data_.at(j + 1) = model_data.at(j);
				//}
			}

			hit_group_shader_table_.AddShaderTables(tables);
			hit_group_shader_table_.Generate(&graphics_device, 
				pipeline_state_.GetStateObjectProperties(), L"HitGroupShaderTable");
		}
	}
}

void Transform::OnGui()
{
	if (ImGui::TreeNode("Transform"))
	{
		ImGui::DragFloat3("Position", &position_.x, 0.01f, -FLT_MAX, FLT_MAX);
		ImGui::DragFloat3("Scaling", &scaling_.x, 0.01f, -FLT_MAX, FLT_MAX);
		ImGui::DragFloat3("Rotation", &rotation_.x, 3.14f / 180.0f * 0.1f, -FLT_MAX, FLT_MAX);
		ImGui::SliderInt("CoordinateSystem", &coordinate_system_index_, 0, 3);
		ImGui::Text(kCoordinateSystemStr[coordinate_system_index_].c_str());
		ImGui::TreePop();
	}
}
