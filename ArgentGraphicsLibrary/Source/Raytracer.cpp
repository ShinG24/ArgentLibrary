#include "../Inc/Raytracer.h"

#include <unordered_set>
#include <vector>


#include "../Inc/GraphicsDevice.h"
#include "../Inc/GraphicsCommandList.h"
#include "../Inc/CommandQueue.h"
#include "../Inc/Fence.h"

#include "../Inc/ShaderCompiler.h"


#define _ALIGNMENT_(value, alignment_size)	 (((value) + (alignment_size)-1) & ~((alignment_size)-1))
namespace argent::graphics
{
	void Raytracer::Awake(const GraphicsDevice& graphics_device, GraphicsCommandList& command_list,
		const CommandQueue& command_queue, Fence& fence)
	{
		CreateAS(graphics_device, command_list, command_queue, fence);
		CreatePipeline(graphics_device);
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

		//Compile the shader library.
		ShaderCompiler compiler;
		compiler.CompileShaderLibrary(L"RayGen.hlsl", ray_gen_library_.ReleaseAndGetAddressOf());;
		compiler.CompileShaderLibrary(L"Miss.hlsl", miss_library_.ReleaseAndGetAddressOf());
		compiler.CompileShaderLibrary(L"Hit.hlsl", hit_library_.ReleaseAndGetAddressOf());

		//Create Root signature
		CreateRayGenSignature(graphics_device);
		CreateHitSignature(graphics_device);
		CreateMissSignature(graphics_device);

		//Create State Object and properties
		CreateStateObjectAndProperties(graphics_device);
	}

	void Raytracer::CreateRayGenSignature(const GraphicsDevice& graphics_device)
	{
		//Need 2 Resources for the RayGen Shader.
		//The One is output buffer in uav register space,
		//the other is top level acceleration structure in srv register space.
		D3D12_DESCRIPTOR_RANGE range[2];

		//For output buffer range
		//UAV range
		range[0].BaseShaderRegister = 0u;
		range[0].NumDescriptors = 1u;
		range[0].RegisterSpace = 0u;
		range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		range[0].OffsetInDescriptorsFromTableStart = 0u;

		//For Top Level Acceleration Structure
		//SRV range
		range[1].BaseShaderRegister = 0u;
		range[1].NumDescriptors = 1;
		range[1].RegisterSpace = 0;
		range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range[1].OffsetInDescriptorsFromTableStart = 0u;

		D3D12_ROOT_PARAMETER root_parameters[1];
		root_parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		root_parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		root_parameters[0].DescriptorTable.NumDescriptorRanges = 2u;
		root_parameters[0].DescriptorTable.pDescriptorRanges = range;


		D3D12_ROOT_SIGNATURE_DESC root_signature_desc{};
		root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
		root_signature_desc.NumParameters = 1u;
		root_signature_desc.pParameters = root_parameters;


		graphics_device.SerializeAndCreateRootSignature(root_signature_desc, ray_gen_signature_.ReleaseAndGetAddressOf(), 
			D3D_ROOT_SIGNATURE_VERSION_1_0);
	}

	void Raytracer::CreateHitSignature(const GraphicsDevice& graphics_device)
	{
		//At this time, hit shader do not need any resource.
		D3D12_ROOT_PARAMETER root_parameter[1];
		root_parameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		root_parameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		root_parameter[0].Descriptor.RegisterSpace = 0u;
		root_parameter[0].Descriptor.ShaderRegister = 0u;

		D3D12_ROOT_SIGNATURE_DESC root_signature_desc{};
		root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
		root_signature_desc.NumParameters = 1u;
		root_signature_desc.pParameters = root_parameter;

		graphics_device.SerializeAndCreateRootSignature(root_signature_desc, 
			hit_signature_.ReleaseAndGetAddressOf(), D3D_ROOT_SIGNATURE_VERSION_1_0);

	}

	void Raytracer::CreateMissSignature(const GraphicsDevice& graphics_device)
	{
		//Same as hit signature
		D3D12_ROOT_PARAMETER root_parameter[1];
		root_parameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		root_parameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		root_parameter[0].Descriptor.RegisterSpace = 0u;
		root_parameter[0].Descriptor.ShaderRegister = 0u;

		D3D12_ROOT_SIGNATURE_DESC root_signature_desc{};
		root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
		root_signature_desc.NumParameters = 1u;
		root_signature_desc.pParameters = root_parameter;

		graphics_device.SerializeAndCreateRootSignature(root_signature_desc, 
			miss_signature_.ReleaseAndGetAddressOf(), D3D_ROOT_SIGNATURE_VERSION_1_0);
	}

	void Raytracer::CreateStateObjectAndProperties(const GraphicsDevice& graphics_device)
	{
		constexpr UINT64 library_counts = 3u;
		constexpr UINT64 hit_group_counts = 1u;
		constexpr UINT64 root_signature_associations_counts = 3u;
		//Pipeline need subobjects counts
		//DXIL libraries counts + hit group count + some other things.
		UINT64 subobject_counts = 
			library_counts +								//DXIL libraries counts
			hit_group_counts +							//Hit group declarations
			1 +											//Shader configuration
			1 +											//Shader Payload
			2 * root_signature_associations_counts +	//Root Signature declaration + association 
			2 +											//Empty global and local root signatures
			1;											//Final pipeline subobject


		std::vector<D3D12_STATE_SUBOBJECT> subobjects(subobject_counts);
		UINT current_subobject_index = 0u;

		//Add all the Subobjects

		//ADD DXIL libraries
		{
			//Ray Gen
			{
				ray_gen_library_data_.exported_symbols_.push_back(L"RayGen");
				ray_gen_library_data_.export_desc_.resize(1u);
				ray_gen_library_data_.export_desc_[0].Name = L"RayGen";
				ray_gen_library_data_.export_desc_[0].ExportToRename = nullptr;
				ray_gen_library_data_.export_desc_[0].Flags = D3D12_EXPORT_FLAG_NONE;

				ray_gen_library_data_.library_desc_.DXILLibrary.BytecodeLength = ray_gen_library_->GetBufferSize();
				ray_gen_library_data_.library_desc_.DXILLibrary.pShaderBytecode = ray_gen_library_->GetBufferPointer();
				ray_gen_library_data_.library_desc_.NumExports = 1u;
				ray_gen_library_data_.library_desc_.pExports = ray_gen_library_data_.export_desc_.data();

				D3D12_STATE_SUBOBJECT subobject{};

				subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
				subobject.pDesc = &ray_gen_library_data_.library_desc_;
				subobjects[current_subobject_index] = subobject;
				++current_subobject_index;
			}

			//Miss
			{
				miss_library_data_.exported_symbols_.push_back(L"Miss");

				miss_library_data_.export_desc_.resize(1u);
				miss_library_data_.export_desc_[0].Name = L"Miss";
				miss_library_data_.export_desc_[0].ExportToRename = nullptr;
				miss_library_data_.export_desc_[0].Flags = D3D12_EXPORT_FLAG_NONE;

				miss_library_data_.library_desc_.DXILLibrary.BytecodeLength = ray_gen_library_->GetBufferSize();
				miss_library_data_.library_desc_.DXILLibrary.pShaderBytecode = ray_gen_library_->GetBufferPointer();
				miss_library_data_.library_desc_.NumExports = 1u;
				miss_library_data_.library_desc_.pExports = miss_library_data_.export_desc_.data();

				D3D12_STATE_SUBOBJECT subobject{};

				subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
				subobject.pDesc = &miss_library_data_.library_desc_;
				subobjects[current_subobject_index] = subobject;
				++current_subobject_index;
			}

			//Hit
			{
				hit_library_data_.exported_symbols_.push_back(L"ClosestHit");

				hit_library_data_.export_desc_.resize(1u);
				hit_library_data_.export_desc_[0].Name = L"Miss";
				hit_library_data_.export_desc_[0].ExportToRename = nullptr;
				hit_library_data_.export_desc_[0].Flags = D3D12_EXPORT_FLAG_NONE;

				hit_library_data_.library_desc_.DXILLibrary.BytecodeLength = ray_gen_library_->GetBufferSize();
				hit_library_data_.library_desc_.DXILLibrary.pShaderBytecode = ray_gen_library_->GetBufferPointer();
				hit_library_data_.library_desc_.NumExports = 1u;
				hit_library_data_.library_desc_.pExports = hit_library_data_.export_desc_.data();

				D3D12_STATE_SUBOBJECT subobject{};

				subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
				subobject.pDesc = &hit_library_data_.library_desc_;
				subobjects[current_subobject_index] = subobject;
				++current_subobject_index;
			}
		}

		//Add hit group declarations
		{
			hit_group_desc_.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
			hit_group_desc_.HitGroupExport = L"HitGroup";
			hit_group_desc_.ClosestHitShaderImport = L"ClosestHit";
			hit_group_desc_.AnyHitShaderImport = nullptr;
			hit_group_desc_.IntersectionShaderImport = nullptr;

			D3D12_STATE_SUBOBJECT subobject{};
			subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
			subobject.pDesc = &hit_group_desc_;
			subobjects[current_subobject_index] = subobject;
			++current_subobject_index;
		}

		//Add a subojbect for the shader payload configuration
		D3D12_RAYTRACING_SHADER_CONFIG shader_config{};
		{
			shader_config.MaxPayloadSizeInBytes = sizeof(float) * 4; //RGB + distance
			shader_config.MaxAttributeSizeInBytes = sizeof(float) * 2; //barycentric coordinates

			D3D12_STATE_SUBOBJECT subobject{};
			subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
			subobject.pDesc = &shader_config;
			subobjects[current_subobject_index] = subobject;
			++current_subobject_index;
		}

		D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shader_payload_association{};
		{
			std::vector<std::wstring> exported_symbols{};
			BuildShaderExportList(exported_symbols);

			std::vector<LPCWSTR> exported_symbol_pointers{};
			exported_symbol_pointers.reserve(exported_symbols.size());
			for(const auto& name : exported_symbols)
			{
				exported_symbol_pointers.push_back(name	.c_str());
			}

			const WCHAR** shader_exports = exported_symbol_pointers.data();

			shader_payload_association.NumExports = static_cast<UINT>(exported_symbols.size());
			shader_payload_association.pExports = shader_exports;

			//Raytracing Shader Config
			shader_payload_association.pSubobjectToAssociate = &subobjects[current_subobject_index - 1];
			
			D3D12_STATE_SUBOBJECT subobject{};
			subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
			subobject.pDesc = &shader_payload_association;
			subobjects[current_subobject_index] = subobject;
			++current_subobject_index;
		}

		//root signature association
		//requires two objects for each : one to declare the root signature
		//and another to associate that root signature to a set of symbols
		{
			//Ray Gen
			{
				ray_gen_association_.root_signature_ = ray_gen_signature_.Get();
				ray_gen_association_.root_signature_pointer_ = ray_gen_signature_.Get();
				ray_gen_association_.symbols_.push_back(L"RayGen");
				ray_gen_association_.symbol_pointers_.push_back(L"RayGen");

				D3D12_STATE_SUBOBJECT root_signature_subobject{};
				root_signature_subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
				root_signature_subobject.pDesc = ray_gen_association_.root_signature_;
				subobjects[current_subobject_index] = root_signature_subobject;
				++current_subobject_index;

				ray_gen_association_.association_.NumExports = ray_gen_association_.symbol_pointers_.size();
				ray_gen_association_.association_.pExports = ray_gen_association_.symbol_pointers_.data();
				ray_gen_association_.association_.pSubobjectToAssociate = &subobjects[current_subobject_index - 1];

				D3D12_STATE_SUBOBJECT root_signature_association_subobject{};
				root_signature_association_subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
				root_signature_association_subobject.pDesc = &ray_gen_association_.association_;

				subobjects[current_subobject_index] = root_signature_association_subobject;
				++current_subobject_index;
			}

			//Miss
			{
				miss_association_.root_signature_ = miss_signature_.Get();
				miss_association_.root_signature_pointer_ = miss_signature_.Get();
				miss_association_.symbols_.push_back(L"RayGen");
				miss_association_.symbol_pointers_.push_back(L"RayGen");

				D3D12_STATE_SUBOBJECT root_signature_subobject{};
				root_signature_subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
				root_signature_subobject.pDesc = miss_association_.root_signature_;
				subobjects[current_subobject_index] = root_signature_subobject;
				++current_subobject_index;

				miss_association_.association_.NumExports = miss_association_.symbol_pointers_.size();
				miss_association_.association_.pExports = miss_association_.symbol_pointers_.data();
				miss_association_.association_.pSubobjectToAssociate = &subobjects[current_subobject_index - 1];

				D3D12_STATE_SUBOBJECT root_signature_association_subobject{};
				root_signature_association_subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
				root_signature_association_subobject.pDesc = &miss_association_.association_;

				subobjects[current_subobject_index] = root_signature_association_subobject;
				++current_subobject_index;

			}

			//Hit
			{
				hit_association_.root_signature_ = hit_signature_.Get();
				hit_association_.root_signature_pointer_ = hit_signature_.Get();
				hit_association_.symbols_.push_back(L"RayGen");
				hit_association_.symbol_pointers_.push_back(L"RayGen");

				D3D12_STATE_SUBOBJECT root_signature_subobject{};
				root_signature_subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
				root_signature_subobject.pDesc = hit_association_.root_signature_;
				subobjects[current_subobject_index] = root_signature_subobject;
				++current_subobject_index;

				hit_association_.association_.NumExports = hit_association_.symbol_pointers_.size();
				hit_association_.association_.pExports = hit_association_.symbol_pointers_.data();
				hit_association_.association_.pSubobjectToAssociate = &subobjects[current_subobject_index - 1];

				D3D12_STATE_SUBOBJECT root_signature_association_subobject{};
				root_signature_association_subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
				root_signature_association_subobject.pDesc = &hit_association_.association_;

				subobjects[current_subobject_index] = root_signature_association_subobject;
				++current_subobject_index;

			}
		}
		
		//The pipeline construction always requires an empty global and local root signature

		//Dummy space for empty global root signature
		ID3D12RootSignature* global_root_signature = dummy_global_root_signature_.Get();
		{
			D3D12_STATE_SUBOBJECT subobject{};
			subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
			subobject.pDesc = &global_root_signature;
			subobjects[current_subobject_index] = subobject;
			++current_subobject_index;
		}

		//Dummy space for empty local root signature
		ID3D12RootSignature* local_root_signature = dummy_local_root_signature_.Get();
		{
			D3D12_STATE_SUBOBJECT subobject{};
			subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
			subobject.pDesc = &local_root_signature;
			subobjects[current_subobject_index] = subobject;
			++current_subobject_index;
		}

		//Raytracing pipeline configuration
		D3D12_RAYTRACING_PIPELINE_CONFIG pipeline_config{};
		{
			pipeline_config.MaxTraceRecursionDepth = 1u;

			D3D12_STATE_SUBOBJECT subobject;
			subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
			subobject.pDesc = &pipeline_config;

			subobjects[current_subobject_index] = subobject;
			++current_subobject_index;
		}

		//Describe the raytracing pipeline state object;
		D3D12_STATE_OBJECT_DESC state_object_desc{};
		state_object_desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
		state_object_desc.NumSubobjects = current_subobject_index;
		state_object_desc.pSubobjects = subobjects.data();


		HRESULT hr = graphics_device.GetLatestDevice()->CreateStateObject(&state_object_desc, IID_PPV_ARGS(raytracing_state_object_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Raytracing State Object");


		hr = raytracing_state_object_->QueryInterface(IID_PPV_ARGS(raytracing_state_object_properties_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Query Raytracing State Properties");

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
}
