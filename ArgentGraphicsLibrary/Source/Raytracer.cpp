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

		transforms_[Plane].position_.y = -5.0f;
		transforms_[Plane].scaling_ = DirectX::XMFLOAT3(50.0f, 1.0f, 50.0f);
		transforms_[Cube].position_.x = 3.0f;
		transforms_[SphereAABB].position_.z = 10.0f;

		materials_[Polygon].albedo_color_ = float4(1.0f, 0.0f, 0.0f, 1.0f);
		materials_[Polygon].diffuse_coefficient_ = 1.0f;
		materials_[Polygon].specular_coefficient_ = 0.2f;
		materials_[Polygon].specular_power_ = 50.f;
		materials_[Polygon].reflectance_coefficient_ = 0.4f;

		materials_[Plane].albedo_color_ = float4(1.0f, 1.0f, 1.0f, 1.0f);
		materials_[Plane].diffuse_coefficient_ = 0.2f;
		materials_[Plane].specular_coefficient_ = 0.6f;
		materials_[Plane].specular_power_ = 50.f;
		materials_[Plane].reflectance_coefficient_ = 0.8f;

		materials_[Cube].albedo_color_ = float4(0.0f, 0.8f, 0.0f, 1.0f);
		materials_[Cube].diffuse_coefficient_ = 0.3f;
		materials_[Cube].specular_coefficient_ = 0.6f;
		materials_[Cube].specular_power_ = 50.f;
		materials_[Cube].reflectance_coefficient_ = 1.0f;

		materials_[SphereAABB].albedo_color_ = float4(1.0f, 1.0f, 1.0f, 1.0f);
		materials_[SphereAABB].diffuse_coefficient_ = 0.2f;
		materials_[SphereAABB].specular_coefficient_ = 0.9f;
		materials_[SphereAABB].specular_power_ = 50.f;
		materials_[SphereAABB].reflectance_coefficient_ = 1.0f;


		CreateAS(graphics_device, command_list, command_queue);
		CreatePipeline(graphics_device);
		CreateOutputBuffer(graphics_device, width, height);
		CreateShaderResourceHeap(graphics_device, cbv_srv_uav_descriptor_heap);
		CreateShaderBindingTable(graphics_device);
	}

	void Raytracer::Update(GraphicsCommandList* graphics_command_list, CommandQueue* upload_command_queue)
	{
		for(int i = 0; i < GeometryTypeCount; ++i)
		{
			if(ImGui::TreeNode(name[i].c_str()))
			{
				transforms_[i].OnGui();
				materials_[i].OnGui();
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

			vertex_buffers_[Polygon] = std::make_unique<VertexBuffer>(&graphics_device, vertices, sizeof(Vertex), 3);
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

			vertex_buffers_[Plane] = std::make_unique<VertexBuffer>(&graphics_device, vertices1, sizeof(Vertex), 6u);
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

			vertex_buffers_[Cube] = std::make_unique<VertexBuffer>(&graphics_device, vertices, sizeof(Vertex), 24u);
			index_buffers_[Cube] = std::make_unique<IndexBuffer>(&graphics_device, indices, 36u);

			D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			desc.Buffer.NumElements = vertex_buffers_[Cube]->GetVertexCounts();
			desc.Buffer.StructureByteStride = sizeof(Vertex);
			graphics_device.GetDevice()->CreateShaderResourceView(vertex_buffers_[Cube]->GetBufferObject(), &desc, cube_vertex_descriptor_.cpu_handle_);
			
			//desc.Format = DXGI_FORMAT_R32_TYPELESS;
			desc.Buffer.FirstElement = 0u;
			desc.Buffer.NumElements = index_buffers_[Cube]->GetIndexCounts();

			desc.Buffer.StructureByteStride = 4;

			graphics_device.GetDevice()->CreateShaderResourceView(index_buffers_[Cube]->GetBufferObject(), &desc, cube_index_descriptor_.cpu_handle_);
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
			bool triangle = i != SphereAABB ? true : false;
			dxr::BLASBuildDesc build_desc;
			build_desc.vertex_buffer_vec_.emplace_back(vertex_buffers_[i].get());
			if(i == Cube)
				build_desc.index_buffer_vec_.emplace_back(index_buffers_[i].get());
			
			unique_id[i] = as_manager_.AddBottomLevelAS(&graphics_device, &command_list, &build_desc, triangle);
		}

		for(int i = 0; i < GeometryTypeCount; ++i)
		{
			DirectX::XMFLOAT4X4 m;
			DirectX::XMStoreFloat4x4(&m, transforms_[i].CalcWorldMatrix());
			tlas_unique_id_[i] = as_manager_.RegisterTopLevelAS(unique_id[i], i, m);
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
			rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, 0u, 1u, 1u);	//For Instance ID
			rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, 1u, 1u, 1u);	//For Material Constant
			rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV, 0u, 1u, 1u);	//For Abledo
			rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV, 1u, 1u, 1u);	//For Normal
			rsc.AddHeapRangesParameter({{2u, 2u, 1u, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0u}});	//For Vertex Buffer and Index Buffer
			hit_local_root_signature_ = rsc.Generate(graphics_device.GetDevice(), true	);
		}

		pipeline.AddHitGroup(L"HitGroup", L"ClosestHit");
		pipeline.AddHitGroup(L"HitGroup1", L"CLHPlane");
		pipeline.AddHitGroup(L"HitGroup2", L"CubeHit");
		pipeline.AddHitGroup(L"HitGroupSphere", L"SphereClosestHit", L"", L"SphereIntersection");

		pipeline.AddRootSignatureAssociation(shared_local_root_signature_.Get(), {L"RayGen"});

		pipeline.AddRootSignatureAssociation(shared_local_root_signature_.Get(), {L"Miss"});
		pipeline.AddRootSignatureAssociation(hit_local_root_signature_.Get(), { {L"HitGroup"}, {L"HitGroup1"}});
		pipeline.AddRootSignatureAssociation(hit_local_root_signature_.Get(), { {L"HitGroup2"}});
		pipeline.AddRootSignatureAssociation(hit_local_root_signature_.Get(), { {L"HitGroupSphere"}});

		pipeline.SetMaxPayloadSize(sizeof(RayPayload) / 4 * sizeof(float));
		pipeline.SetMaxAttributeSize(3 * sizeof(float));
		pipeline.SetMaxRecursionDepth(_MAX_RECURSION_DEPTH_);

		raytracing_state_object_ = pipeline.Generate(dummy_global_root_signature_.Get(), dummy_local_root_signature_.Get());
		HRESULT hr = raytracing_state_object_->QueryInterface(IID_PPV_ARGS(raytracing_state_object_properties_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Query Interface");
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
			uint entry_size = shader_record_size + 8 * RootSignatureBinderCount;
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

			map += shader_record_size;

			std::vector<void*> data(RootSignatureBinderCount);

			data.at(ObjectCbv) = reinterpret_cast<void*>(world_matrix_buffer_->GetGPUVirtualAddress() + sizeof(ObjectConstant) * Plane);
			data.at(MaterialCbv) = reinterpret_cast<void*>(material_buffer_->GetGPUVirtualAddress() + sizeof(Material) * Plane);
			data.at(VertexBufferGpuDescriptorHandle) = reinterpret_cast<void*>(0);
			memcpy(map, data.data(), data.size() * 8);

			map += entry_size - shader_record_size;

			id = raytracing_state_object_properties_->GetShaderIdentifier(L"HitGroup2");
			memcpy(map, id, shader_record_size);
			map += shader_record_size;

			//Not Entry directly
			//memcpy(map, reinterpret_cast<void*>(cube_vertex_descriptor_.gpu_handle.ptr), 8) does not
			//act my assumption.
			data.at(ObjectCbv) = reinterpret_cast<void*>(world_matrix_buffer_->GetGPUVirtualAddress() + sizeof(ObjectConstant) * Cube);
			data.at(MaterialCbv) = reinterpret_cast<void*>(material_buffer_->GetGPUVirtualAddress() + sizeof(Material) * Cube);
			data.at(VertexBufferGpuDescriptorHandle) = reinterpret_cast<void*>(cube_vertex_descriptor_.gpu_handle_.ptr);
			//data.at(1) = reinterpret_cast<void*>(object_descriptor_.gpu_handle_.ptr);

			//Map Resource
			memcpy(map, data.data(), data.size() * 8);

			map += entry_size - shader_record_size;

			id = raytracing_state_object_properties_->GetShaderIdentifier(L"HitGroupSphere");
			memcpy(map, id, shader_record_size);

			map += shader_record_size;
			data.at(MaterialCbv) = reinterpret_cast<void*>(material_buffer_->GetGPUVirtualAddress() + sizeof(Material) * SphereAABB);
			data.at(ObjectCbv) = reinterpret_cast<void*>(world_matrix_buffer_->GetGPUVirtualAddress() + sizeof(ObjectConstant) * SphereAABB);
			memcpy(map, data.data(), data.size() * 8);
			map += entry_size - shader_record_size;

			hit_shader_table_->Unmap(0u, nullptr);
		}
	}
}
