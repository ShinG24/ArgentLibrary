#include "../Inc/Raytracer.h"

#include <unordered_set>
#include <vector>

#include "../External/Imgui/imgui.h"

#include "../Inc/GraphicsCommon.h"
#include "../Inc/GraphicsDevice.h"
#include "../Inc/GraphicsCommandList.h"
#include "../Inc/CommandQueue.h"

#include "../Inc/ShaderCompiler.h"


#include "../../Common.hlsli"


#define _USE_SBT_GENERATOR_	0

#define _USE_BLAS_GENERATOR_ 0




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

		CreateAS(graphics_device, command_list, command_queue);
		CreatePipeline(graphics_device);
		CreateOutputBuffer(graphics_device, width, height);
		CreateShaderResourceHeap(graphics_device, cbv_srv_uav_descriptor_heap);
		CreateShaderBindingTable(graphics_device);
	}

	void Raytracer::Update(GraphicsCommandList* graphics_command_list, CommandQueue* upload_command_queue)
	{
		graphics_command_list->Activate();
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
		XMMATRIX S = XMMatrixScaling(cube_transform_.scaling_.x, cube_transform_.scaling_.y, cube_transform_.scaling_.z);
		XMMATRIX R = XMMatrixRotationRollPitchYaw(cube_transform_.rotation_.x, cube_transform_.rotation_.y, cube_transform_.rotation_.z);
		XMMATRIX T = XMMatrixTranslation(cube_transform_.position_.x, cube_transform_.position_.y, cube_transform_.position_.z);
		XMMATRIX M = S * R * T;

		instances_.at(2).second = M;

		top_level_as_generator_.Generate(graphics_command_list->GetCommandList4(), top_level_as_buffer_.pScratch.Get(),
			top_level_as_buffer_.pResult.Get(), top_level_as_buffer_.pInstanceDesc.Get(),
			true, top_level_as_buffer_.pResult.Get());

		graphics_command_list->Deactivate();
		ID3D12CommandList* command_lists[]{ graphics_command_list->GetCommandList() };
		upload_command_queue->Execute(1u, command_lists);
		upload_command_queue->Signal();
		upload_command_queue->WaitForGpu();
	}

	void Raytracer::OnRender(const GraphicsCommandList& graphics_command_list, D3D12_GPU_DESCRIPTOR_HANDLE scene_constant_gpu_handle)
	{
		auto command_list = graphics_command_list.GetCommandList4();

		graphics_command_list.SetTransitionBarrier(output_buffer_.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		D3D12_DISPATCH_RAYS_DESC desc{};

#if _USE_SBT_GENERATOR_
		uint32_t ray_generation_section_size_in_bytes = sbt_generator_.GetRayGenSectionSize();
		desc.RayGenerationShaderRecord.StartAddress = sbt_storage_->GetGPUVirtualAddress();
		desc.RayGenerationShaderRecord.SizeInBytes = ray_generation_section_size_in_bytes;

		uint32_t miss_section_size_in_bytes = sbt_generator_.GetMissSectionSize();
		desc.MissShaderTable.StartAddress = sbt_storage_->GetGPUVirtualAddress() + ray_generation_section_size_in_bytes;
		desc.MissShaderTable.SizeInBytes = miss_section_size_in_bytes;
		desc.MissShaderTable.StrideInBytes = sbt_generator_.GetMissEntrySize();

		uint32_t hit_group_section_size = sbt_generator_.GetHitGroupSectionSize();
		desc.HitGroupTable.StartAddress = sbt_storage_->GetGPUVirtualAddress() +
			ray_generation_section_size_in_bytes + miss_section_size_in_bytes;
		//desc.HitGroupTable.StartAddress = _ALIGNMENT_(desc.HitGroupTable.StartAddress, 64);
		desc.HitGroupTable.SizeInBytes = hit_group_section_size;
		desc.HitGroupTable.StrideInBytes = sbt_generator_.GetHitGroupEntrySize();

#else

		desc.RayGenerationShaderRecord.StartAddress = raygen_shader_table_->GetGPUVirtualAddress();
		desc.RayGenerationShaderRecord.SizeInBytes = 32u;

		desc.MissShaderTable.StartAddress = miss_shader_table_->GetGPUVirtualAddress();
		desc.MissShaderTable.SizeInBytes = 32u;
		desc.MissShaderTable.StrideInBytes = 32u;

		desc.HitGroupTable.StartAddress = hit_shader_table_->GetGPUVirtualAddress();
		desc.HitGroupTable.SizeInBytes = 192u;
		desc.HitGroupTable.StrideInBytes = 64u;

#endif

		desc.Width = static_cast<UINT>(width_);
		desc.Height = height_;
		desc.Depth = _MAX_REFLECTION_;

		//Set Global Raytracing RootSignature Resource
		{
			command_list->SetComputeRootSignature(dummy_global_root_signature_.Get());
			//OutputBuffer & TLAS
			command_list->SetComputeRootDescriptorTable(0u, output_descriptor_.gpu_handle_);
			command_list->SetComputeRootDescriptorTable(1u, scene_constant_gpu_handle);
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
				 {{0.0f, 0.6f, 0.5f}, {}},
	        {{0.25f, -0.6f, 0.5f}, {}},
	        {{-0.25f, -0.6f, 0.5f}, {}}
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
	}

	void Raytracer::CreateAS(const GraphicsDevice& graphics_device, 
	                         GraphicsCommandList& command_list, CommandQueue& command_queue)
	{
		BuildGeometry(graphics_device);

#if _USE_BLAS_GENERATOR_
		AccelerationStructureBuffers bottom_level_buffer = 
		CreateBottomLevelAs(graphics_device, command_list.GetCommandList4(), 
			{{vertex_buffer_.Get(), 3}});
		AccelerationStructureBuffers bottom_level_buffer1 = CreateBottomLevelAs(graphics_device, command_list.GetCommandList4(),
			{  {vertex_buffer1_.Get(), 6} });

		AccelerationStructureBuffers bottom_level_buffer2/* = CreateBottomLevelAs(graphics_device, command_list.GetCommandList4(), 
			{{vertex_buffer2_.Get(), 24}})*/;
		{
			nv_helpers_dx12::BottomLevelASGenerator bottom_level_as;

			bottom_level_as.AddVertexBuffer(vertex_buffer2_.Get(), 0u, 24,
					sizeof(Vertex), index_buffer_.Get(), 0u, 36, nullptr, 0u);
			
			UINT64 scratch_size_in_bytes = 0u;
			UINT64 result_size_in_bytes = 0u;

			bottom_level_as.ComputeASBufferSizes(graphics_device.GetLatestDevice(), 
				false, &scratch_size_in_bytes, &result_size_in_bytes);

			AccelerationStructureBuffers buffers;
			graphics_device.CreateBuffer(kDefaultHeapProp, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				static_cast<UINT>(scratch_size_in_bytes), D3D12_RESOURCE_STATE_COMMON,buffers.pScratch.ReleaseAndGetAddressOf());
			graphics_device.CreateBuffer(kDefaultHeapProp, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				static_cast<UINT>(result_size_in_bytes), D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
				buffers.pResult.ReleaseAndGetAddressOf());

			bottom_level_as.Generate(command_list.GetCommandList4(), buffers.pScratch.Get(), 
				buffers.pResult.Get(), false, nullptr);
			bottom_level_buffer2 = buffers;
		}
#else

		bottom_level_0_ = std::make_unique<BottomLevelAccelerationStructure>(&graphics_device,
			&command_list, vertex_buffer0_.get());

		bottom_level_1_ = std::make_unique<BottomLevelAccelerationStructure>(&graphics_device,
			&command_list, vertex_buffer1_.get());

		BLASBuildDesc build_desc{};
		build_desc.vertex_buffer_vec_ = { vertex_buffer2_.get() };
		build_desc.index_buffer_vec_ = { index_buffer_.get() };

		bottom_level_2_ = std::make_unique<BottomLevelAccelerationStructure>(&graphics_device,
			&command_list, &build_desc);

#endif
		DirectX::XMFLOAT3 pos{ 0.0f, -3.0f, 0.0f };
		DirectX::XMFLOAT3 scale{ 100.0f, 100.0f, 100.0f };

		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

		DirectX::XMFLOAT3 pos1{3.0f, 0.0f, 0.0f};
		DirectX::XMMATRIX T1 = DirectX::XMMatrixTranslation(pos1.x, pos1.y, pos1.z);

#if _USE_BLAS_GENERATOR_
		instances_ = {{bottom_level_buffer.pResult, XMMatrixIdentity() }, { bottom_level_buffer1.pResult, S * T},{ bottom_level_buffer2.pResult, T1}, };
#else
		instances_ = {{bottom_level_0_->GetResultBuffer(), XMMatrixIdentity() },
			{ bottom_level_1_->GetResultBuffer(), S * T},
			{ bottom_level_2_->GetResultBuffer(), T1}, };
#endif
		CreateTopLevelAs(graphics_device, command_list.GetCommandList4(), instances_);

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
#if 0
			D3D12_ROOT_SIGNATURE_DESC root_signature_desc{};
			root_signature_desc.NumParameters = 0u;
			root_signature_desc.pParameters = nullptr;
			root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

			graphics_device.SerializeAndCreateRootSignature(root_signature_desc, 
				dummy_global_root_signature_.ReleaseAndGetAddressOf(), D3D_ROOT_SIGNATURE_VERSION_1);

			root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
			graphics_device.SerializeAndCreateRootSignature(root_signature_desc, 
				dummy_local_root_signature_.ReleaseAndGetAddressOf(), D3D_ROOT_SIGNATURE_VERSION_1);
#else
			nv_helpers_dx12::RootSignatureGenerator rsc;

			rsc.AddHeapRangesParameter(
				{
					{0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
					{0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
				//	{0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
				}
			);

			rsc.AddHeapRangesParameter({{0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND}});
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
#endif
		}

		nv_helpers_dx12::RayTracingPipelineGenerator pipeline(graphics_device.GetLatestDevice());

		//Compile the shader library.
		ShaderCompiler shader_compiler;
		shader_compiler.CompileShaderLibrary(L"RayGen.hlsl", ray_gen_library_.ReleaseAndGetAddressOf());;
		shader_compiler.CompileShaderLibrary(L"Miss.hlsl", miss_library_.ReleaseAndGetAddressOf());
		shader_compiler.CompileShaderLibrary(L"Hit.hlsl", hit_library_.ReleaseAndGetAddressOf());
		shader_compiler.CompileShaderLibrary(L"Plane.hlsl", hit1_library_.ReleaseAndGetAddressOf());
		shader_compiler.CompileShaderLibrary(L"CubeCLH.hlsl", hit2_library_.ReleaseAndGetAddressOf());

		pipeline.AddLibrary(ray_gen_library_.Get(), {L"RayGen"});
		pipeline.AddLibrary(miss_library_.Get(), {L"Miss"});
		pipeline.AddLibrary(hit_library_.Get(), {L"ClosestHit"});
		pipeline.AddLibrary(hit1_library_.Get(), {L"CLHPlane"});
		pipeline.AddLibrary(hit2_library_.Get(), {L"CubeHit"});

		//Shared root signature
		{
			nv_helpers_dx12::RootSignatureGenerator rsc;
			shared_local_root_signature_ = rsc.Generate(graphics_device.GetDevice(), true);
		}

		{
			nv_helpers_dx12::RootSignatureGenerator rsc;
			rsc.AddHeapRangesParameter({{0u, 2u, 1u, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0u}});
			hit_local_root_signature_ = rsc.Generate(graphics_device.GetDevice(), true	);
		}

		pipeline.AddHitGroup(L"HitGroup", L"ClosestHit");
		pipeline.AddHitGroup(L"HitGroup1", L"CLHPlane");
		pipeline.AddHitGroup(L"HitGroup2", L"CubeHit");

		pipeline.AddRootSignatureAssociation(shared_local_root_signature_.Get(), {L"RayGen"});

#if 1
		pipeline.AddRootSignatureAssociation(shared_local_root_signature_.Get(), {L"Miss"});
		pipeline.AddRootSignatureAssociation(shared_local_root_signature_.Get(), { {L"HitGroup"}, {L"HitGroup1"}});
		pipeline.AddRootSignatureAssociation(hit_local_root_signature_.Get(), { {L"HitGroup2"}});

#else

		pipeline.AddRootSignatureAssociation(miss_signature_.Get(), {L"Miss"});
		pipeline.AddRootSignatureAssociation(hit_signature_.Get(), { {L"HitGroup"}, {L"HitGroup1"}});

#endif
	//	pipeline.AddRootSignatureAssociation(hit1_signature_.Get(), {L"HitGroup1"});

		pipeline.SetMaxPayloadSize(sizeof(RayPayload) / 4 * sizeof(float));
		//pipeline.SetMaxPayloadSize(5 * sizeof(float));
		pipeline.SetMaxAttributeSize(2 * sizeof(float));
		pipeline.SetMaxRecursionDepth(_MAX_REFLECTION_);

		raytracing_state_object_ = pipeline.Generate(dummy_global_root_signature_.Get(), dummy_local_root_signature_.Get());
		HRESULT hr = raytracing_state_object_->QueryInterface(IID_PPV_ARGS(raytracing_state_object_properties_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Query Interface");
	}

	void Raytracer::BuildShaderExportList(std::vector<std::wstring> exported_symbols)
	{
		//Build a list containing the export symbols for the ray generation shaders,
		//miss shaders, and hit group name

		//Get all names from libraries
		//Get names associated to hit groups
		//Return list of libraries + hit group name - shaders in hit groups

		std::unordered_set<std::wstring> exports;

		//In truth need to check that no name is exported more than once
		//But in this I do not code.
		exports.insert(ray_gen_library_data_.exported_symbols_.at(0));
		exports.insert(miss_library_data_.exported_symbols_.at(0));
		exports.insert(hit_library_data_.exported_symbols_.at(0));

		//In truth it's need to verify that the hit groups do not reference an
		//unknown shader name, but i do not code.

		exports.erase(L"ClosestHit");
		exports.insert(L"HitGroup");

		for(const auto& name : exports)
		{
			exported_symbols.push_back(name);
		}
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
#if 0

		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
		uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		graphics_device.GetDevice()->CreateUnorderedAccessView(output_buffer_.Get(), 
			nullptr, &uav_desc, srv_handle);

#else

		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
		uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		graphics_device.GetDevice()->CreateUnorderedAccessView(output_buffer_.Get(), 
			nullptr, &uav_desc, output_descriptor_.cpu_handle_);

#endif

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
		srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.RaytracingAccelerationStructure.Location = top_level_as_buffer_.pResult->GetGPUVirtualAddress();


#if 0
		srv_handle.ptr += graphics_device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		graphics_device.GetDevice()->CreateShaderResourceView(nullptr, &srv_desc,
			srv_handle);

#else
		graphics_device.GetDevice()->CreateShaderResourceView(nullptr, &srv_desc,
			tlas_result_descriptor_.cpu_handle_);

#endif
	}

	void Raytracer::CreateShaderBindingTable(const GraphicsDevice& graphics_device)
	{

#if _USE_SBT_GENERATOR_
		{
			sbt_generator_.Reset();

			//D3D12_GPU_DESCRIPTOR_HANDLE srv_uav_heap_handle = descriptor_heap_->GetGPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE srv_uav_heap_handle = output_descriptor_.gpu_handle_;

			//auto heap_pointer = reinterpret_cast<UINT64**>(srv_uav_heap_handle.ptr);

	#if 0 
			auto heap_pointer1 = reinterpret_cast<UINT64**>(tlas_result_descriptor_.gpu_handle_.ptr);
			auto heap_pointer2 = reinterpret_cast<UINT64**>(scene_constant_buffer_.GetGpuHandle(0).ptr);

			sbt_generator_.AddRayGenerationProgram(L"RayGen", {{heap_pointer}, { heap_pointer1 }, { heap_pointer2 } });
	#else
			sbt_generator_.AddRayGenerationProgram(L"RayGen", {});
	#endif

			sbt_generator_.AddMissProgram(L"Miss", {});
			sbt_generator_.AddHitGroup(L"HitGroup", {/*{(void*)(vertex_buffer_->GetGPUVirtualAddress())},*/ });
			sbt_generator_.AddHitGroup(L"HitGroup1", {/*{(void*)(vertex_buffer1_->GetGPUVirtualAddress())}*/});
			sbt_generator_.AddHitGroup(L"HitGroup2", {{(void*)(cube_vertex_descriptor_.gpu_handle_.ptr)}});
			//sbt_generator_.AddHitGroup(L"HitGroup2", {{(void*)(vertex_buffer2_->GetGPUVirtualAddress())}});


			UINT sbt_size = sbt_generator_.ComputeSBTSize();

			D3D12_RESOURCE_DESC res_desc{};
			res_desc.Alignment = 0u;
			res_desc.DepthOrArraySize = 1u;
			res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
			res_desc.Format = DXGI_FORMAT_UNKNOWN;
			res_desc.Height = 1u;
			res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			res_desc.MipLevels = 1u;
			res_desc.SampleDesc.Count = 1u;
			res_desc.SampleDesc.Quality = 0u;
			res_desc.Width = sbt_size;

			//HRESULT hr = graphics_device.GetDevice()->CreateCommittedResource(&kUploadHeapProp,
			//	D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, 
			//	IID_PPV_ARGS(sbt_storage_.ReleaseAndGetAddressOf()));
			//_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create SBT Storage");
			graphics_device.CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, sbt_size, D3D12_RESOURCE_STATE_GENERIC_READ, sbt_storage_.ReleaseAndGetAddressOf());


			sbt_generator_.Generate(sbt_storage_.Get(), raytracing_state_object_properties_.Get());
			
		}

#else
	//	return;
		//Try
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
				uint num_hit_group = 3;
				uint entry_size = shader_record_size + 8u;
				entry_size = _ALIGNMENT_(entry_size, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
				uint resource_size = entry_size * num_hit_group;
				//resource_size = _ALIGNMENT_(resource_size, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
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
				std::vector<void*> data(1);
				data.at(0) = reinterpret_cast<void*>(cube_vertex_descriptor_.gpu_handle_.ptr);

				//Map Resource
				memcpy(map, data.data(), data.size() * 8);

				hit_shader_table_->Unmap(0u, nullptr);
			}
		}

#endif
	}

	AccelerationStructureBuffers Raytracer::CreateBottomLevelAs(
		const GraphicsDevice& graphics_device,
		ID3D12GraphicsCommandList4* command_list,
		std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vertex_buffers)
	{
		nv_helpers_dx12::BottomLevelASGenerator bottom_level_as;
		for(const auto& buffer : vertex_buffers)
		{
			bottom_level_as.AddVertexBuffer(buffer.first.Get(), 0u, buffer.second,
				sizeof(Vertex), 0, 0u);
		}

		UINT64 scratch_size_in_bytes = 0u;
		UINT64 result_size_in_bytes = 0u;

		bottom_level_as.ComputeASBufferSizes(graphics_device.GetLatestDevice(), 
			false, &scratch_size_in_bytes, &result_size_in_bytes);

		AccelerationStructureBuffers buffers;
		graphics_device.CreateBuffer(kDefaultHeapProp, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			static_cast<UINT>(scratch_size_in_bytes), D3D12_RESOURCE_STATE_COMMON,buffers.pScratch.ReleaseAndGetAddressOf());
		graphics_device.CreateBuffer(kDefaultHeapProp, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			static_cast<UINT>(result_size_in_bytes), D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
			buffers.pResult.ReleaseAndGetAddressOf());

		bottom_level_as.Generate(command_list, buffers.pScratch.Get(), 
			buffers.pResult.Get(), false, nullptr);
		return buffers;
	}

	void Raytracer::CreateTopLevelAs(const GraphicsDevice& graphics_device,
		ID3D12GraphicsCommandList4* command_list,
		const std::vector<std::pair<ComPtr<ID3D12Resource>, XMMATRIX>>& instances)
	{
		for(size_t i = 0; i < instances.size(); ++i)
		{
			top_level_as_generator_.AddInstance(instances[i].first.Get(), 
				instances[i].second, static_cast<UINT>(i), static_cast<UINT>(i));
		}

		top_level_as_generator_.ComputeASBufferSizes(graphics_device.GetLatestDevice(), 
			true, &tlas_scratch_size_,
			&tlas_result_size_, &tlas_instance_size_);


		graphics_device.CreateBuffer(kDefaultHeapProp, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			static_cast<UINT>(tlas_scratch_size_), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, top_level_as_buffer_.pScratch.ReleaseAndGetAddressOf());
		
		graphics_device.CreateBuffer(kDefaultHeapProp, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			static_cast<UINT>(tlas_result_size_), D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			top_level_as_buffer_.pResult.ReleaseAndGetAddressOf());
		
		graphics_device.CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, 
			static_cast<UINT>(tlas_instance_size_), D3D12_RESOURCE_STATE_GENERIC_READ,
			top_level_as_buffer_.pInstanceDesc.ReleaseAndGetAddressOf());

		top_level_as_generator_.Generate(command_list, 
			top_level_as_buffer_.pScratch.Get(), 
			top_level_as_buffer_.pResult.Get(),
			top_level_as_buffer_.pInstanceDesc.Get());
	}
}
