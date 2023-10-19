#include "../Inc/Raytracer.h"

#include <vector>

#include "../RaytracingPipelineGenerator.h"
#include "../RootSignatureGenerator.h"

#include "../External/Imgui/imgui.h"

#include "../Inc/GraphicsCommon.h"
#include "../Inc/GraphicsDevice.h"
#include "../Inc/GraphicsCommandList.h"
#include "../Inc/CommandQueue.h"

#include "../Inc/ShaderCompiler.h"


#include "../../Common.hlsli"


#define _USE_SBT_GENERATOR_	0

namespace argent::graphics
{
	void Raytracer::Awake(const GraphicsDevice& graphics_device, GraphicsCommandList& command_list,
		CommandQueue& command_queue, UINT64 width, UINT height, 
			DescriptorHeap& cbv_srv_uav_descriptor_heap)
	{
		width_ = width;
		height_ = height;

		cube_vertex_descriptor_ = cbv_srv_uav_descriptor_heap.PopDescriptor();
		cube_index_descriptor_ = cbv_srv_uav_descriptor_heap.PopDescriptor();
		object_descriptor_ = cbv_srv_uav_descriptor_heap.PopDescriptor();

		CreateAS(graphics_device, command_list, command_queue);
		CreatePipeline(graphics_device);
		CreateOutputBuffer(graphics_device, width, height);
		CreateShaderResourceHeap(graphics_device, cbv_srv_uav_descriptor_heap);
		CreateShaderBindingTable(graphics_device);
	}

	void Raytracer::Update(GraphicsCommandList* graphics_command_list, CommandQueue* upload_command_queue)
	{
		//Imgui
		{
			if(ImGui::TreeNode("Cube"))
			{
				ImGui::DragFloat3("Position", &cube_transform_.position_.x, 0.01f, -FLT_MAX, FLT_MAX);
				ImGui::DragFloat3("Scaling", &cube_transform_.scaling_.x, 0.01f, -FLT_MAX, FLT_MAX);
				ImGui::DragFloat3("Rotation", &cube_transform_.rotation_.x, 3.14f / 180.0f * 0.1f, -FLT_MAX, FLT_MAX);
				ImGui::TreePop();
			}
		}
		graphics_command_list->Activate();
		XMMATRIX S = XMMatrixScaling(cube_transform_.scaling_.x, cube_transform_.scaling_.y, cube_transform_.scaling_.z);
		XMMATRIX R = XMMatrixRotationRollPitchYaw(cube_transform_.rotation_.x, cube_transform_.rotation_.y, cube_transform_.rotation_.z);
		XMMATRIX T = XMMatrixTranslation(cube_transform_.position_.x, cube_transform_.position_.y, cube_transform_.position_.z);
		XMMATRIX M = S * R * T;

		top_level_acceleration_structure_.SetMatrix(M, 2u);
		top_level_acceleration_structure_.Update(graphics_command_list);


		graphics_command_list->Deactivate();
		ID3D12CommandList* command_lists[]{ graphics_command_list->GetCommandList() };
		upload_command_queue->Execute(1u, command_lists);
		upload_command_queue->Signal();
		upload_command_queue->WaitForGpu();

		{
			DirectX::XMFLOAT4X4* map;
			object_world_buffer_->Map(0u, nullptr, reinterpret_cast<void**>(&map));

			DirectX::XMFLOAT4X4 m;
			DirectX::XMStoreFloat4x4(&m, M);
			memcpy(map, &m, sizeof(DirectX::XMFLOAT4X4));

			object_world_buffer_->Unmap(0u, nullptr);
		}
	}

	void Raytracer::OnRender(const GraphicsCommandList& graphics_command_list, D3D12_GPU_VIRTUAL_ADDRESS scene_constant_gpu_handle)
	{
		auto command_list = graphics_command_list.GetCommandList4();

		graphics_command_list.SetTransitionBarrier(output_buffer_.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		D3D12_DISPATCH_RAYS_DESC desc{};

		desc.RayGenerationShaderRecord.StartAddress = raygen_shader_table_->GetGPUVirtualAddress();
		desc.RayGenerationShaderRecord.SizeInBytes = 32u;

		desc.MissShaderTable.StartAddress = miss_shader_table_->GetGPUVirtualAddress();
		desc.MissShaderTable.SizeInBytes = 32u;
		desc.MissShaderTable.StrideInBytes = 32u;

		desc.HitGroupTable.StartAddress = hit_shader_table_->GetGPUVirtualAddress();
		desc.HitGroupTable.SizeInBytes = hit_shader_table_size_;
		desc.HitGroupTable.StrideInBytes = hit_shader_table_stride_;
		desc.Width = static_cast<UINT>(width_);
		desc.Height = height_;
		desc.Depth = 1;

		//Set Global Raytracing RootSignature Resource
		{
			command_list->SetComputeRootSignature(dummy_global_root_signature_.Get());
			//OutputBuffer & TLAS
			command_list->SetComputeRootDescriptorTable(0u, output_descriptor_.gpu_handle_);
			command_list->SetComputeRootConstantBufferView(1u, scene_constant_gpu_handle);
		}

		command_list->SetPipelineState1(raytracing_state_object_.Get());
		command_list->DispatchRays(&desc);

		graphics_command_list.SetTransitionBarrier(output_buffer_.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	}

	void Raytracer::BuildGeometry(const GraphicsDevice& graphics_device)
	{
		//Polygon
		{
			Vertex vertices[3]
			{
				 {{0.0f, 3.0f, 0}, {}},
	        {{0.0f, -3.0f, 3.0f}, {}},
	        {{-0.0f, -3.0f, -3.0f}, {}}
			};

			vertex_buffer0_ = std::make_unique<VertexBuffer>(&graphics_device, vertices, sizeof(Vertex), 3);
		}

		//Plane
		{
			Vertex vertices1[6]
			{
				{{ -1.0f, 0.0f, 1.0f }, {}},
				{{ 1.0f, 0.0f, 1.0f }, {}},
				{{ -1.0f, 0.0f, -1.0f }, {}},

				{{ -1.0f, 0.0f, -1.0f }, {}},
				{{ 1.0f, 0.0f, 1.0f }, {}},
				{{ 1.0f, 0.0f, -1.0f }, {}},
			};

			vertex_buffer1_ = std::make_unique<VertexBuffer>(&graphics_device, vertices1, sizeof(Vertex), 6u);
		}

		//Cube
		{
			UINT32 indices[] =
		    {
		        3,1,0,
		        2,1,3,

		        6,4,5,
		        7,4,6,

		        11,9,8,
		        10,9,11,

		        14,12,13,
		        15,12,14,

		        19,17,16,
		        18,17,19,

		        22,20,21,
		        23,20,22
		    };

		    // Cube vertices positions and corresponding triangle normals.
		    Vertex vertices[] =
		    {
		        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

		        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },

		        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

		        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

		        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },

		        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		    };

			vertex_buffer2_ = std::make_unique<VertexBuffer>(&graphics_device, vertices, sizeof(Vertex), 24u);
			index_buffer_ = std::make_unique<IndexBuffer>(&graphics_device, indices, 36u);

			D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			desc.Buffer.NumElements = vertex_buffer2_->GetVertexCounts();
			desc.Buffer.StructureByteStride = sizeof(Vertex);
			graphics_device.GetDevice()->CreateShaderResourceView(vertex_buffer2_->GetBufferObject(), &desc, cube_vertex_descriptor_.cpu_handle_);
			
			//desc.Format = DXGI_FORMAT_R32_TYPELESS;
			desc.Buffer.FirstElement = 0u;
			desc.Buffer.NumElements = index_buffer_->GetIndexCounts();

			desc.Buffer.StructureByteStride = 4;

			graphics_device.GetDevice()->CreateShaderResourceView(index_buffer_->GetBufferObject(), &desc, cube_index_descriptor_.cpu_handle_);
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

			aabb_vertex_buffer_ = std::make_unique<VertexBuffer>(&graphics_device, 
				&rt_aabb, sizeof(D3D12_RAYTRACING_AABB), 1u);
		}
	}

	void Raytracer::CreateAS(const GraphicsDevice& graphics_device, 
	                         GraphicsCommandList& command_list, CommandQueue& command_queue)
	{
		BuildGeometry(graphics_device);

		bottom_level_0_ = std::make_unique<dxr::BottomLevelAccelerationStructure>(&graphics_device,
			&command_list, vertex_buffer0_.get());

		bottom_level_1_ = std::make_unique<dxr::BottomLevelAccelerationStructure>(&graphics_device,
			&command_list, vertex_buffer1_.get());


		dxr::BLASBuildDesc build_desc{};
		build_desc.vertex_buffer_vec_ = { vertex_buffer2_.get() };
		build_desc.index_buffer_vec_ = { index_buffer_.get() };

		bottom_level_2_ = std::make_unique<dxr::BottomLevelAccelerationStructure>(&graphics_device,
			&command_list, &build_desc);

		bottom_level_sphere_ = std::make_unique<dxr::BottomLevelAccelerationStructure>(&graphics_device,
			&command_list, aabb_vertex_buffer_.get(), nullptr, false);

		DirectX::XMFLOAT3 pos{ 0.0f, -3.0f, 0.0f };
		DirectX::XMFLOAT3 scale{ 100.0f, 100.0f, 100.0f };

		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

		DirectX::XMFLOAT3 pos1{3.0f, 0.0f, 0.0f};
		DirectX::XMMATRIX T1 = DirectX::XMMatrixTranslation(pos1.x, pos1.y, pos1.z);


		top_level_acceleration_structure_.AddInstance(bottom_level_0_.get(), XMMatrixIdentity(),
			0u);
		top_level_acceleration_structure_.AddInstance(bottom_level_1_.get(), S * T,
			1u);
		top_level_acceleration_structure_.AddInstance(bottom_level_2_.get(), T1,
			2u);

		top_level_acceleration_structure_.AddInstance(bottom_level_sphere_.get(), XMMatrixIdentity(),
			3u);

		//top_level_acceleration_structure_.AddInstance(bottom_level_0_.get(), XMMatrixIdentity(),
		//	3u);

		top_level_acceleration_structure_.Generate(&graphics_device, &command_list);

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
		//Create Dummy Root Signature
		{
			nv_helpers_dx12::RootSignatureGenerator rsc;

			rsc.AddHeapRangesParameter(
				{
					{0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
					{0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
				//	{0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
				}
			);

			rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, 0u, 0u, 1u);
			//rsc.AddHeapRangesParameter({{0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND}});
			//rsc.AddHeapRangesParameter(
			//	{
			//		{ 1, 2u, 0u, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND}
			//});

			dummy_global_root_signature_ = rsc.Generate(graphics_device.GetDevice(), false);


			D3D12_ROOT_SIGNATURE_DESC root_signature_desc{};
			root_signature_desc.NumParameters = 0u;
			root_signature_desc.pParameters = nullptr;

			root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
			graphics_device.SerializeAndCreateRootSignature(root_signature_desc, 
				dummy_local_root_signature_.ReleaseAndGetAddressOf(), D3D_ROOT_SIGNATURE_VERSION_1);
		}

		nv_helpers_dx12::RayTracingPipelineGenerator pipeline(graphics_device.GetLatestDevice());

		//Compile the shader library.
		ShaderCompiler shader_compiler;
		shader_compiler.CompileShaderLibrary(L"RayGen.hlsl", ray_gen_library_.ReleaseAndGetAddressOf());;
		shader_compiler.CompileShaderLibrary(L"Miss.hlsl", miss_library_.ReleaseAndGetAddressOf());
		shader_compiler.CompileShaderLibrary(L"Hit.hlsl", hit_library_.ReleaseAndGetAddressOf());
		shader_compiler.CompileShaderLibrary(L"Plane.hlsl", hit1_library_.ReleaseAndGetAddressOf());
		shader_compiler.CompileShaderLibrary(L"CubeCLH.hlsl", hit2_library_.ReleaseAndGetAddressOf());

		//Sphere Intersection Demo
		shader_compiler.CompileShaderLibrary(L"SphereClosestHit.hlsl", sphere_closest_hit_library_.ReleaseAndGetAddressOf());
		shader_compiler.CompileShaderLibrary(L"SphereIntersection.hlsl", sphere_intersection_library_.ReleaseAndGetAddressOf());

		pipeline.AddLibrary(ray_gen_library_.Get(), {L"RayGen"});
		pipeline.AddLibrary(miss_library_.Get(), {L"Miss"});
		pipeline.AddLibrary(hit_library_.Get(), {L"ClosestHit"});
		pipeline.AddLibrary(hit1_library_.Get(), {L"CLHPlane"});
		pipeline.AddLibrary(hit2_library_.Get(), {L"CubeHit"});

		pipeline.AddLibrary(sphere_closest_hit_library_.Get(), {L"SphereClosestHit"});
		pipeline.AddLibrary(sphere_intersection_library_.Get(), {L"SphereIntersection"});

		//Shared root signature
		{
			nv_helpers_dx12::RootSignatureGenerator rsc;
			shared_local_root_signature_ = rsc.Generate(graphics_device.GetDevice(), true);
		}

		{
			nv_helpers_dx12::RootSignatureGenerator rsc;
			rsc.AddHeapRangesParameter({{0u, 2u, 1u, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0u}});
			rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, 0u, 1u, 1u);
		//	rsc.AddHeapRangesParameter({{0u, 1u, 1u, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0u}});
			hit_local_root_signature_ = rsc.Generate(graphics_device.GetDevice(), true	);
		}

		pipeline.AddHitGroup(L"HitGroup", L"ClosestHit");
		pipeline.AddHitGroup(L"HitGroup1", L"CLHPlane");
		pipeline.AddHitGroup(L"HitGroup2", L"CubeHit");
		pipeline.AddHitGroup(L"HitGroupSphere", L"SphereClosestHit", L"", L"SphereIntersection");

		pipeline.AddRootSignatureAssociation(shared_local_root_signature_.Get(), {L"RayGen"});

		pipeline.AddRootSignatureAssociation(shared_local_root_signature_.Get(), {L"Miss"});
		pipeline.AddRootSignatureAssociation(shared_local_root_signature_.Get(), { {L"HitGroup"}, {L"HitGroup1"}});
		pipeline.AddRootSignatureAssociation(hit_local_root_signature_.Get(), { {L"HitGroup2"}});
		pipeline.AddRootSignatureAssociation(shared_local_root_signature_.Get(), { {L"HitGroupSphere"}});

		pipeline.SetMaxPayloadSize(sizeof(RayPayload) / 4 * sizeof(float));
		pipeline.SetMaxAttributeSize(3 * sizeof(float));
		pipeline.SetMaxRecursionDepth(_MAX_RECURSION_DEPTH_);

		raytracing_state_object_ = pipeline.Generate(dummy_global_root_signature_.Get(), dummy_local_root_signature_.Get());
		HRESULT hr = raytracing_state_object_->QueryInterface(IID_PPV_ARGS(raytracing_state_object_properties_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Query Interface");
	}

	void Raytracer::CreateOutputBuffer(const GraphicsDevice& graphics_device, UINT64 width, UINT height)
	{
		D3D12_RESOURCE_DESC res_desc{};
		res_desc.DepthOrArraySize = 1u;
		res_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		res_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		res_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		res_desc.Width = width;
		res_desc.Height = height;
		res_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		res_desc.MipLevels = 1u;
		res_desc.SampleDesc.Count = 1u;
		HRESULT hr = graphics_device.GetDevice()->CreateCommittedResource(&kDefaultHeapProp, 
			D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, 
			IID_PPV_ARGS(output_buffer_.ReleaseAndGetAddressOf()));

		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create output buffer");
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
		srv_desc.RaytracingAccelerationStructure.Location = top_level_acceleration_structure_.GetResultBuffer()->GetGPUVirtualAddress();

		graphics_device.GetDevice()->CreateShaderResourceView(nullptr, &srv_desc,
			tlas_result_descriptor_.cpu_handle_);


		graphics_device.CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, sizeof(DirectX::XMFLOAT4X4), 
			D3D12_RESOURCE_STATE_GENERIC_READ, object_world_buffer_.ReleaseAndGetAddressOf());

		//srv_desc = {};
		//srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		//srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		//srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		//srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		//srv_desc.Buffer.NumElements = 1u;
		//srv_desc.Buffer.StructureByteStride = sizeof(DirectX::XMFLOAT4X4);
		//graphics_device.GetDevice()->CreateShaderResourceView(object_world_buffer_.Get(), &srv_desc, 
		//	object_descriptor_.cpu_handle_);

		DirectX::XMFLOAT4X4* map;
		object_world_buffer_->Map(0u, nullptr, reinterpret_cast<void**>(&map));

		DirectX::XMFLOAT4X4 m;
		DirectX::XMStoreFloat4x4(&m, DirectX::XMMatrixIdentity());
		memcpy(map, &m, sizeof(DirectX::XMFLOAT4X4));

		object_world_buffer_->Unmap(0u, nullptr);
	}

	void Raytracer::CreateShaderBindingTable(const GraphicsDevice& graphics_device)
	{

		//The size Shader Identifier
		uint shader_record_size =  D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;

		//Raygen Table
		{
			graphics_device.CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, shader_record_size, D3D12_RESOURCE_STATE_GENERIC_READ, 
				raygen_shader_table_.ReleaseAndGetAddressOf());

			//Map by 1byte offset
			uint8_t* map;
			raygen_shader_table_->Map(0u, nullptr, reinterpret_cast<void**>(&map));

			auto id = raytracing_state_object_properties_->GetShaderIdentifier(L"RayGen");
			memcpy(map, id, shader_record_size);

			raygen_shader_table_->Unmap(0u, nullptr);
		}

		//Miss
		{
			graphics_device.CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, shader_record_size, D3D12_RESOURCE_STATE_GENERIC_READ, 
				miss_shader_table_.ReleaseAndGetAddressOf());

			uint8_t* map;
			miss_shader_table_->Map(0u, nullptr, reinterpret_cast<void**>(&map));

			auto id = raytracing_state_object_properties_->GetShaderIdentifier(L"Miss");
			memcpy(map, id, shader_record_size);

			miss_shader_table_->Unmap(0u, nullptr);
		}

		//Hit
		{
			//Need to use only one Entry size.
			//if  hit1 shader use 32bit entry size, hit2 use 64bit, and hit3 use 128bit,
			//then you have to use 128bit for all entry size.
			uint num_hit_group = 4;
			uint entry_size = shader_record_size + 8 * 2;
			entry_size = _ALIGNMENT_(entry_size, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
			hit_shader_table_stride_ = entry_size;
			uint resource_size = entry_size * num_hit_group;
			resource_size = _ALIGNMENT_(resource_size, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
			hit_shader_table_size_ = resource_size;
			graphics_device.CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, resource_size, D3D12_RESOURCE_STATE_GENERIC_READ, 
				hit_shader_table_.ReleaseAndGetAddressOf());

			uint8_t* map;
			hit_shader_table_->Map(0u, nullptr, reinterpret_cast<void**>(&map));

			//Map Shader Identifier
			auto id = raytracing_state_object_properties_->GetShaderIdentifier(L"HitGroup");

			memcpy(map, id, shader_record_size);
			map += entry_size;

			id = raytracing_state_object_properties_->GetShaderIdentifier(L"HitGroup1");
			memcpy(map, id, shader_record_size);
			map += entry_size;

			id = raytracing_state_object_properties_->GetShaderIdentifier(L"HitGroup2");
			memcpy(map, id, shader_record_size);
			map += shader_record_size;

			//Not Entry directly
			//memcpy(map, reinterpret_cast<void*>(cube_vertex_descriptor_.gpu_handle.ptr), 8) does not
			//act my assumption.
			std::vector<void*> data(2);
			data.at(0) = reinterpret_cast<void*>(cube_vertex_descriptor_.gpu_handle_.ptr);
			data.at(1) = reinterpret_cast<void*>(object_world_buffer_->GetGPUVirtualAddress());
			//data.at(1) = reinterpret_cast<void*>(object_descriptor_.gpu_handle_.ptr);

			//Map Resource
			memcpy(map, data.data(), data.size() * 8);

			map += entry_size - shader_record_size;

			id = raytracing_state_object_properties_->GetShaderIdentifier(L"HitGroupSphere");
			memcpy(map, id, shader_record_size);
			map += entry_size;

			hit_shader_table_->Unmap(0u, nullptr);
		}
	}
}
