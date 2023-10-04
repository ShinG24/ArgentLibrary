#include "../Inc/Raytracer.h"

#include "../Inc/GraphicsDevice.h"
#include "../Inc/GraphicsCommandList.h"
#include "../Inc/CommandQueue.h"
#include "../Inc/Fence.h"


#define _ALIGNMENT_(value, alignment_size)	 (((value) + (alignment_size)-1) & ~((alignment_size)-1))
namespace argent::graphics
{
	void Raytracer::Awake(const GraphicsDevice& graphics_device, GraphicsCommandList& command_list,
		const CommandQueue& command_queue, Fence& fence)
	{
		CreateAS(graphics_device, command_list, command_queue, fence);

	}

	void Raytracer::CreatePipeline(const GraphicsDevice& graphics_device)
	{

	}

	void Raytracer::CreateAS(const GraphicsDevice& graphics_device, 
		GraphicsCommandList& command_list, const CommandQueue& command_queue, 
		Fence& fence)
	{
		CreateBLAS(graphics_device, command_list.GetCommandList4());
		CreateTLAS(graphics_device, command_list.GetCommandList4());

		//Execute Commandlist
		command_list.Deactivate();

		ID3D12CommandList* command_lists[]
		{
			command_list.GetCommandList(),
		};
		command_queue.Execute(1u, command_lists);

		fence.PutUpFence(command_queue);
		fence.WaitForGpuInCurrentFrame();
	}

	void Raytracer::CreateBLAS(const GraphicsDevice& graphics_device, ID3D12GraphicsCommandList4* command_list)
	{
		Vertex vertices[3]
		{
			 {{0.0f, 0.6, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
        {{0.25f, -0.6f, 0.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},
        {{-0.25f, -0.6f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}}
		};

		graphics_device.CreateVertexBufferAndView(sizeof(Vertex), 3, 
			vertex_buffer_.ReleaseAndGetAddressOf(), vertex_buffer_view_);

		D3D12_RAYTRACING_GEOMETRY_DESC geometry_desc{};
		geometry_desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
		geometry_desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometry_desc.Triangles.VertexBuffer.StartAddress = vertex_buffer_->GetGPUVirtualAddress();
		geometry_desc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
		geometry_desc.Triangles.VertexCount = 3;
		geometry_desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geometry_desc.Triangles.IndexBuffer = 0;
		geometry_desc.Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;
		geometry_desc.Triangles.Transform3x4 = 0;

		UINT64 scratch_size_in_bytes = 0;
		UINT64 result_size_in_bytes = 0;

		//Compute buffer size
		{
			//If allow update specify the flag allow_update
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags =
				D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS build_input{};
			build_input.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			build_input.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			build_input.NumDescs = 1u;
			build_input.pGeometryDescs = &geometry_desc;
			build_input.Flags = flags;

			//Building the acceleration structure requires some scratch space,
			//as well ass space to store the resulting structure.
			//This function computes a conservative estimate of the memory requirements for both,
			//based on the geometry size.
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info{};
			graphics_device.GetLatestDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&build_input, &info);

			//Buffer sizes need to be 256-bytes-aligned
			scratch_size_in_bytes = _ALIGNMENT_(info.ScratchDataSizeInBytes,
				D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			result_size_in_bytes = _ALIGNMENT_(info.ResultDataMaxSizeInBytes, 
				D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		}

		//Create scratch_buffer
		{
			D3D12_HEAP_PROPERTIES heap_prop{};
			heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
			heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heap_prop.CreationNodeMask = 0u;
			heap_prop.VisibleNodeMask = 0u;

			D3D12_RESOURCE_DESC desc{};
			desc.Alignment = 0u;
			desc.DepthOrArraySize = 1u;
			desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.Height = 1u;
			desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			desc.MipLevels = 1u;
			desc.SampleDesc.Count = 1u;
			desc.SampleDesc.Quality = 0u;
			desc.Width = scratch_size_in_bytes;


			HRESULT hr = graphics_device.GetDevice()->CreateCommittedResource(&heap_prop,
				D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, 
				nullptr, IID_PPV_ARGS(blas_scratch_buffer_.ReleaseAndGetAddressOf()));
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Scrach Buffer");
		}

		//Create Result Buffer
		{
			D3D12_HEAP_PROPERTIES heap_prop{};
			heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
			heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heap_prop.CreationNodeMask = 0u;
			heap_prop.VisibleNodeMask = 0u;

			D3D12_RESOURCE_DESC desc{};
			desc.Alignment = 0u;
			desc.DepthOrArraySize = 1u;
			desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.Height = 1u;
			desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			desc.MipLevels = 1u;
			desc.SampleDesc.Count = 1u;
			desc.SampleDesc.Quality = 0u;
			desc.Width = result_size_in_bytes;

			HRESULT hr = graphics_device.GetDevice()->CreateCommittedResource(&heap_prop,
				D3D12_HEAP_FLAG_NONE, &desc, 
				D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
				nullptr, IID_PPV_ARGS(blas_result_buffer_.ReleaseAndGetAddressOf()));
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Scrach Buffer");
		}

		//Generate
		{
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc{};
			build_desc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			build_desc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;	
			build_desc.Inputs.NumDescs = 1u;
			build_desc.Inputs.pGeometryDescs = &geometry_desc;
			build_desc.DestAccelerationStructureData = blas_result_buffer_->GetGPUVirtualAddress();
			build_desc.ScratchAccelerationStructureData = blas_scratch_buffer_->GetGPUVirtualAddress();
			build_desc.SourceAccelerationStructureData = 0u;
			build_desc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

			command_list->BuildRaytracingAccelerationStructure(&build_desc, 0u, 
				nullptr);

			D3D12_RESOURCE_BARRIER resource_barrier{};
			resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			resource_barrier.UAV.pResource = blas_result_buffer_.Get();
			resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			command_list->ResourceBarrier(1u, &resource_barrier);
		}

		blas_scratch_buffer_->SetName(L"BLAS Scratch");
		blas_result_buffer_->SetName(L"BLAS Result");
	}

	void Raytracer::CreateTLAS(const GraphicsDevice& graphics_device, ID3D12GraphicsCommandList4* command_list)
	{
		struct Instance
		{
			ID3D12Resource* bottom_level_as_;	//result buffer
			const DirectX::XMMATRIX& transform_;
			UINT instance_id_;
			UINT hit_group_index_;
		};

		const DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity();

		Instance instance{ blas_result_buffer_.Get(), transform, 0, 0 };

		UINT64 scratch_size, result_size, instance_desc_size;

		//Compute Buffer Size
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags;
		{
			flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS build_input{};
			build_input.Flags = flags;
			build_input.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			build_input.NumDescs = 1u;
			build_input.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

			//Pre Build
			//Check necessary heap size
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO build_info{};
			graphics_device.GetLatestDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&build_input, &build_info);

			//Alignment buffer size to 256-bytes;
			build_info.ResultDataMaxSizeInBytes = _ALIGNMENT_(build_info.ResultDataMaxSizeInBytes, 
				D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			build_info.ScratchDataSizeInBytes = _ALIGNMENT_(build_info.ScratchDataSizeInBytes, 
				D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

			result_size = build_info.ResultDataMaxSizeInBytes;
			scratch_size = build_info.ScratchDataSizeInBytes;

			instance_desc_size = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * 1u;
			instance_desc_size = _ALIGNMENT_(instance_desc_size, 
				D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		}

		HRESULT hr{};
		//Create scratch buffer
		D3D12_HEAP_PROPERTIES heap_prop{};
		D3D12_RESOURCE_DESC res_desc{};
		{
			heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
			heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heap_prop.CreationNodeMask = 0u;
			heap_prop.VisibleNodeMask = 0u;

			res_desc.Alignment = 0u;
			res_desc.DepthOrArraySize = 1u;
			res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			res_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			res_desc.Format = DXGI_FORMAT_UNKNOWN;
			res_desc.Height = 1u;
			res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			res_desc.MipLevels = 1u;
			res_desc.SampleDesc.Count = 1u;
			res_desc.SampleDesc.Quality = 0u;
			res_desc.Width = scratch_size;
			hr = graphics_device.GetDevice()->CreateCommittedResource(&heap_prop, 
				D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 
				nullptr, IID_PPV_ARGS(tlas_scratch_buffer_.ReleaseAndGetAddressOf()));
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create TLAS scratch bufer");
		}

		//Create result buffer
		{
			res_desc.Width = result_size;
			hr = graphics_device.GetDevice()->CreateCommittedResource(&heap_prop, 
				D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
				nullptr	, IID_PPV_ARGS(tlas_result_buffer_.ReleaseAndGetAddressOf()));
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create TLAS result bufer");
		}

		//Create instance desc
		{
			heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
			res_desc.Width = instance_desc_size;
			res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
			hr = graphics_device.GetDevice()->CreateCommittedResource(&heap_prop, 
				D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, 
				nullptr, IID_PPV_ARGS(tlas_instance_buffer_.ReleaseAndGetAddressOf()));
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create TLAS instance bufer");
		}

		tlas_scratch_buffer_->SetName(L"TLAS Scratch");
		tlas_result_buffer_->SetName(L"TLAS Result");
		tlas_instance_buffer_->SetName(L"TLAS Instance");
		//Generate
		{
			bool updateonly = false;
			D3D12_RAYTRACING_INSTANCE_DESC* instance_desc{};
			tlas_instance_buffer_->Map(0, nullptr, reinterpret_cast<void**>(&instance_desc));

			if(!instance_desc)
			{
				_ASSERT_EXPR(FALSE, L"Cannot map the instance descriptor buffer");
			}

			auto instance_count = 1u;

			ZeroMemory(instance_desc, instance_desc_size);

			instance_desc[0].InstanceID = instance.instance_id_;
			instance_desc[0].InstanceContributionToHitGroupIndex = instance.hit_group_index_;
			instance_desc[0].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
			memcpy(instance_desc[0].Transform, &instance.transform_, sizeof(instance_desc[0].Transform));
			instance_desc[0].AccelerationStructure = instance.bottom_level_as_->GetGPUVirtualAddress();
			instance_desc[0].InstanceMask = 0xFF;

			tlas_instance_buffer_->Unmap(0u, nullptr);

			auto p_source_as = 0;


			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc{};
			build_desc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
			build_desc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			build_desc.Inputs.InstanceDescs = tlas_instance_buffer_->GetGPUVirtualAddress();
			build_desc.Inputs.NumDescs = 1u;
			build_desc.DestAccelerationStructureData = tlas_result_buffer_->GetGPUVirtualAddress();
			build_desc.ScratchAccelerationStructureData = tlas_scratch_buffer_->GetGPUVirtualAddress();
			build_desc.SourceAccelerationStructureData = 0u;
			build_desc.Inputs.Flags = flags;

			command_list->BuildRaytracingAccelerationStructure(&build_desc, 
				0, nullptr);

			D3D12_RESOURCE_BARRIER resource_barrier{};
			resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			resource_barrier.UAV.pResource = tlas_result_buffer_.Get();
			resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			command_list->ResourceBarrier(1, &resource_barrier);
		}
	}
}
