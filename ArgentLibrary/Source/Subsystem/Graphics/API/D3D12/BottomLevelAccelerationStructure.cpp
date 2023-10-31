#include "Subsystem/Graphics/API/D3D12/BottomLevelAccelerationStructure.h"

#include "Subsystem/Graphics/API/D3D12/D3D12Common.h"
#include "Subsystem/Graphics/API/D3D12/GraphicsDevice.h"
#include "Subsystem/Graphics/API/D3D12/GraphicsCommandList.h"
#include "Subsystem/Graphics/API/D3D12/VertexBuffer.h"
#include "Subsystem/Graphics/API/D3D12/IndexBuffer.h"

#include "Subsystem/Graphics/Common/GraphicsCommon.h"


namespace argent::graphics::dx12
{
	BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(const GraphicsDevice* graphics_device,
		const GraphicsCommandList* graphics_command_list, const BLASBuildDesc* build_desc, UINT unique_id,
		bool is_triangle):
		unique_id_(unique_id)
	{
		std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometry_desc(build_desc->vertex_buffer_vec_.size());

		if(build_desc->vertex_buffer_vec_.size() != build_desc->index_buffer_vec_.size() &&
			build_desc->index_buffer_vec_.size() != 0)
		{
			_ASSERT_EXPR(FALSE, L"BottomLevelAccelerationStrcutre Creation Error!\n"
					   "vertex_buffer_vec.size() and index_buffer_vec.size() need to be same value or "
						"index_buffer_vec.size() need to be 0");
		}

		for(size_t i = 0; i < geometry_desc.size(); ++i)
		{
			auto& desc = geometry_desc.at(i);
			const auto& v = build_desc->vertex_buffer_vec_.at(i);

			desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
			//desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

			//TODO Geometry Type does not need to change the same bottom level??
			desc.Type = is_triangle ? 
			D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES :
			D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;

			if(!is_triangle)
			{
				desc.AABBs.AABBs.StartAddress = v->GetView().BufferLocation;
				desc.AABBs.AABBs.StrideInBytes = v->GetView().StrideInBytes;
				desc.AABBs.AABBCount = 1u;	
			}
			else
			{
				desc.Triangles.VertexBuffer.StartAddress =  v->GetView().BufferLocation;
				desc.Triangles.VertexBuffer.StrideInBytes = v->GetView().StrideInBytes;
				desc.Triangles.VertexCount = v->GetVertexCounts();
				desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;

				if(build_desc->transform_vec_.size() == 0)
				{
					desc.Triangles.Transform3x4 = 0u;
				}
				else
				{
					desc.Triangles.Transform3x4 = build_desc->transform_vec_.at(i);
				}

				if(build_desc->index_buffer_vec_.size() == 0)
				{
					desc.Triangles.IndexBuffer = 0u;
					desc.Triangles.IndexCount = 0u;
					desc.Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;
				}
				else
				{
					const auto& in = build_desc->index_buffer_vec_.at(i);
					desc.Triangles.IndexBuffer = in->GetView().BufferLocation;
					desc.Triangles.IndexCount = in->GetIndexCounts();
					desc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
				}
			}
		}

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS as_input{};
		as_input.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		as_input.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
		as_input.NumDescs = static_cast<UINT>(geometry_desc.size());
		as_input.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		as_input.pGeometryDescs = geometry_desc.data();

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO as_info{};
		graphics_device->GetLatestDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&as_input, &as_info);

		scratch_buffer_size_ = _ALIGNMENT_(as_info.ScratchDataSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		result_buffer_size_ = _ALIGNMENT_(as_info.ResultDataMaxSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		graphics_device->CreateBuffer(kDefaultHeapProp, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, scratch_buffer_size_, 
			D3D12_RESOURCE_STATE_COMMON, scratch_buffer_object_.ReleaseAndGetAddressOf());

		graphics_device->CreateBuffer(kDefaultHeapProp, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, result_buffer_size_, 
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, result_buffer_object_.ReleaseAndGetAddressOf());

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC as_desc{};
		as_desc.DestAccelerationStructureData = result_buffer_object_->GetGPUVirtualAddress();
		as_desc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		as_desc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
		as_desc.Inputs.NumDescs = static_cast<UINT>(geometry_desc.size());
		as_desc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		as_desc.Inputs.pGeometryDescs = geometry_desc.data();
		as_desc.ScratchAccelerationStructureData = scratch_buffer_object_->GetGPUVirtualAddress();
		graphics_command_list->GetCommandList4()->BuildRaytracingAccelerationStructure(&as_desc, 0u, nullptr);
	}
}
