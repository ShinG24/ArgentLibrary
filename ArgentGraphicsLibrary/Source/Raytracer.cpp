#include "../Inc/Raytracer.h"

#include <unordered_set>
#include <vector>

#include "../Inc/GraphicsCommon.h"
#include "../Inc/GraphicsDevice.h"
#include "../Inc/GraphicsCommandList.h"
#include "../Inc/CommandQueue.h"
#include "../Inc/Fence.h"

#include "../Inc/ShaderCompiler.h"


namespace argent::graphics
{
	void Raytracer::Awake(const GraphicsDevice& graphics_device, GraphicsCommandList& command_list,
		const CommandQueue& command_queue, Fence& fence, UINT64 width, UINT height, 
			DescriptorHeap& cbv_srv_uav_descriptor_heap)
	{
		width_ = width;
		height_ = height;

		scene_constant_buffer_.Awake(graphics_device, cbv_srv_uav_descriptor_heap);


		CreateAS(graphics_device, command_list, command_queue, fence);
		CreatePipeline(graphics_device);
		CreateOutputBuffer(graphics_device, width, height);
		CreateShaderResourceHeap(graphics_device, cbv_srv_uav_descriptor_heap);
		CreateShaderBindingTable(graphics_device);

	}

	void Raytracer::OnRender(const GraphicsCommandList& graphics_command_list)
	{
		//Update Camera
		{
			DirectX::XMVECTOR Eye = DirectX::XMLoadFloat4(&camera_position_);
			DirectX::XMVECTOR Focus = {};
			//Focus.m128_f32[2] += 1.0f;
			DirectX::XMVECTOR Up{ 0, 1, 0, 0 };
			auto view = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
			auto proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov_angle_), aspect_ratio_, near_z_, far_z_);

			SceneConstant data{};
			data.camera_position_ = camera_position_;
			DirectX::XMStoreFloat4x4(&data.inv_view_projection_, DirectX::XMMatrixInverse(nullptr, view * proj));

			scene_constant_buffer_.Update(data, 0);
		}
		auto command_list = graphics_command_list.GetCommandList4();

		//std::vector<ID3D12DescriptorHeap*> heaps{ descriptor_heap_.Get() };
		//command_list->SetDescriptorHeaps(1u, heaps.data());

		//return;
		D3D12_RESOURCE_BARRIER resource_barrier{};
		resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		resource_barrier.Transition.pResource = output_buffer_.Get();
		command_list->ResourceBarrier(1u, &resource_barrier);

		D3D12_DISPATCH_RAYS_DESC desc{};
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
		desc.HitGroupTable.SizeInBytes = hit_group_section_size;
		desc.HitGroupTable.StrideInBytes = sbt_generator_.GetHitGroupEntrySize();

		desc.Width = static_cast<UINT>(width_);
		desc.Height = height_;
		desc.Depth = 1;

		command_list->SetPipelineState1(raytracing_state_object_.Get());
		command_list->DispatchRays(&desc);

		resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		resource_barrier.Transition.pResource = output_buffer_.Get();
		command_list->ResourceBarrier(1, &resource_barrier);
	}

	void Raytracer::CreateAS(const GraphicsDevice& graphics_device, 
	                         GraphicsCommandList& command_list, const CommandQueue& command_queue, 
	                         Fence& fence)
	{
		Vertex vertices[3]
		{
			 {{0.0f, 0.6f, 0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}},
        {{0.25f, -0.6f, 0.5f}, {0.0f, 1.0f, 1.0f, 1.0f}},
        {{-0.25f, -0.6f, 0.5f}, {1.0f, 0.0f, 1.0f, 1.0f}}
		};

		graphics_device.CreateVertexBufferAndView(sizeof(Vertex), 3, 
			vertex_buffer_.ReleaseAndGetAddressOf(), vertex_buffer_view_);

		Vertex* map;
		vertex_buffer_->Map(0u, nullptr, reinterpret_cast<void**>(&map));

		map[0] = vertices[0];
		map[1] = vertices[1];
		map[2] = vertices[2];

		vertex_buffer_->Unmap(0u, nullptr);


		Vertex vertices1[4]
		{
			{{ -0.3f, 0.8f, 0.3f }, {}},
			{{ 0.5f, 0.3f, 0.3f }, {}},
			{{ -0.7f, -0.2f, 0.3f }, {}},
			{{ 0.4f, -0.5f, 0.3f}, {}},
		};


		graphics_device.CreateVertexBufferAndView(sizeof(Vertex), 3, 
			vertex_buffer1_.ReleaseAndGetAddressOf(), vertex_buffer_view1_);

		Vertex* map1;
		vertex_buffer1_->Map(0u, nullptr, reinterpret_cast<void**>(&map1));

		map1[0] = vertices1[0];
		map1[1] = vertices1[1];
		map1[2] = vertices1[2];
		map1[3] = vertices1[3];

		vertex_buffer1_->Unmap(0u, nullptr);

		AccelerationStructureBuffers bottom_level_buffer = 
		CreateBottomLevelAs(graphics_device, command_list.GetCommandList4(), 
			{{vertex_buffer_.Get(), 3}});
		AccelerationStructureBuffers bottom_level_buffer1 = CreateBottomLevelAs(graphics_device, command_list.GetCommandList4(),
			{  {vertex_buffer1_.Get(), 4} });


		instances_ = {{bottom_level_buffer.pResult, XMMatrixIdentity() }, { bottom_level_buffer1.pResult, XMMatrixIdentity()}};
		CreateTopLevelAs(graphics_device, command_list.GetCommandList4(), instances_);

		//Execute Command list
		command_list.Deactivate();

		ID3D12CommandList* command_lists[]
		{
			command_list.GetCommandList(),
		};
		command_queue.Execute(1u, command_lists);

		fence.PutUpFence(command_queue);
		fence.WaitForGpuInCurrentFrame();

		bottom_level_as_ = bottom_level_buffer.pResult;
	}

	void Raytracer::CreatePipeline(const GraphicsDevice& graphics_device)
	{
		//Create Dummy Root Signature
		{
			D3D12_ROOT_SIGNATURE_DESC root_signature_desc{};
			root_signature_desc.NumParameters = 0u;
			root_signature_desc.pParameters = nullptr;
			root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

			graphics_device.SerializeAndCreateRootSignature(root_signature_desc, 
				dummy_global_root_signature_.ReleaseAndGetAddressOf(), D3D_ROOT_SIGNATURE_VERSION_1);

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
		shader_compiler.CompileShaderLibrary(L"Hit1.hlsl", hit1_library_.ReleaseAndGetAddressOf());

		pipeline.AddLibrary(ray_gen_library_.Get(), {L"RayGen"});
		pipeline.AddLibrary(miss_library_.Get(), {L"Miss"});
		pipeline.AddLibrary(hit_library_.Get(), {L"ClosestHit"});
		pipeline.AddLibrary(hit1_library_.Get(), {L"ClosestHit1"});

		//RayGen signature
		{
			nv_helpers_dx12::RootSignatureGenerator rsc;

			rsc.AddHeapRangesParameter(
				{
					{0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
					{0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
				//	{0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
				}
			);

			rsc.AddHeapRangesParameter({{0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND}});

			ray_gen_signature_ = rsc.Generate(graphics_device.GetDevice(), true);
		}

		//Miss Signature
		{
			nv_helpers_dx12::RootSignatureGenerator rsc;
			miss_signature_ = rsc.Generate(graphics_device.GetDevice(), true);
		}

		//Hit Signature
		{
			nv_helpers_dx12::RootSignatureGenerator rsc;
			rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV);
			hit_signature_ = rsc.Generate(graphics_device.GetDevice(), true);
		//	hit1_signature_ = rsc.Generate(graphics_device.GetDevice(), true);
		}

		pipeline.AddHitGroup(L"HitGroup", L"ClosestHit");
		pipeline.AddHitGroup(L"HitGroup1", L"ClosestHit1");

		pipeline.AddRootSignatureAssociation(ray_gen_signature_.Get(), {L"RayGen"});
		pipeline.AddRootSignatureAssociation(miss_signature_.Get(), {L"Miss"});
		pipeline.AddRootSignatureAssociation(hit_signature_.Get(), { {L"HitGroup"}, {L"HitGroup1"}});
	//	pipeline.AddRootSignatureAssociation(hit1_signature_.Get(), {L"HitGroup1"});

		pipeline.SetMaxPayloadSize(4 * sizeof(float));
		pipeline.SetMaxAttributeSize(2 * sizeof(float));
		pipeline.SetMaxRecursionDepth(1);

		raytracing_state_object_ = pipeline.Generate();
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
		HRESULT hr = graphics_device.GetDevice()->CreateCommittedResource(&default_heap, 
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
		sbt_generator_.Reset();

		//D3D12_GPU_DESCRIPTOR_HANDLE srv_uav_heap_handle = descriptor_heap_->GetGPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE srv_uav_heap_handle = output_descriptor_.gpu_handle_;

		auto heap_pointer = reinterpret_cast<UINT64**>(srv_uav_heap_handle.ptr);

#if 0 
		auto heap_pointer1 = reinterpret_cast<UINT64**>(tlas_result_descriptor_.gpu_handle_.ptr);
		auto heap_pointer2 = reinterpret_cast<UINT64**>(scene_constant_buffer_.GetGpuHandle(0).ptr);

		sbt_generator_.AddRayGenerationProgram(L"RayGen", {{heap_pointer}, { heap_pointer1 }, { heap_pointer2 } });
#else
		sbt_generator_.AddRayGenerationProgram(L"RayGen", {{heap_pointer}});
#endif
		sbt_generator_.AddMissProgram(L"Miss", {});
		sbt_generator_.AddHitGroup(L"HitGroup", {(void*)(vertex_buffer_->GetGPUVirtualAddress())});
		sbt_generator_.AddHitGroup(L"HitGroup1", {(void*)(vertex_buffer1_->GetGPUVirtualAddress())});


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

		HRESULT hr = graphics_device.GetDevice()->CreateCommittedResource(&upload_heap,
			D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, 
			IID_PPV_ARGS(sbt_storage_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create SBT Storage");


		sbt_generator_.Generate(sbt_storage_.Get(), raytracing_state_object_properties_.Get());

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
		graphics_device.CreateBuffer(default_heap, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			static_cast<UINT>(scratch_size_in_bytes), D3D12_RESOURCE_STATE_COMMON,buffers.pScratch.ReleaseAndGetAddressOf());
		graphics_device.CreateBuffer(default_heap, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, 
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

//		int

		UINT64 scratch_size, result_size, instance_desc_size;
		top_level_as_generator_.ComputeASBufferSizes(graphics_device.GetLatestDevice(), 
			true, &scratch_size, &result_size, &instance_desc_size);


		graphics_device.CreateBuffer(default_heap, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, 
			static_cast<UINT>(scratch_size), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, top_level_as_buffer_.pScratch.ReleaseAndGetAddressOf());
		
		graphics_device.CreateBuffer(default_heap, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, 
			static_cast<UINT>(result_size), D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			top_level_as_buffer_.pResult.ReleaseAndGetAddressOf());
		
		graphics_device.CreateBuffer(upload_heap, D3D12_RESOURCE_FLAG_NONE, 
			static_cast<UINT>(instance_desc_size), D3D12_RESOURCE_STATE_GENERIC_READ, 
			top_level_as_buffer_.pInstanceDesc.ReleaseAndGetAddressOf());

		top_level_as_generator_.Generate(command_list, 
			top_level_as_buffer_.pScratch.Get(), 
			top_level_as_buffer_.pResult.Get(),
			top_level_as_buffer_.pInstanceDesc.Get());
	}
}
