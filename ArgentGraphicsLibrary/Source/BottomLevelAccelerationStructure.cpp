#include "../Inc/BottomLevelAccelerationStructure.h"

#include "../Inc/GraphicsCommandList.h"
#include "../Inc/GraphicsCommon.h"
#include "../Inc/GraphicsDevice.h"

#include "../Inc/VertexBuffer.h"
#include "../Inc/IndexBuffer.h"

namespace argent::graphics
{
	BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(const GraphicsDevice* graphics_device,
		GraphicsCommandList* graphics_command_list, ID3D12Resource* vertex_buffer, UINT vertex_count,
		UINT size_of_vertex_structure, ID3D12Resource* index_buffer, UINT index_count)
	{
		D3D12_RAYTRACING_GEOMETRY_DESC desc{};
		desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		desc.Triangles.VertexBuffer.StartAddress = vertex_buffer->GetGPUVirtualAddress();
		desc.Triangles.VertexBuffer.StrideInBytes = size_of_vertex_structure;
		desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		desc.Triangles.VertexCount = vertex_count;
		desc.Triangles.IndexBuffer = index_buffer ? index_buffer->GetGPUVirtualAddress() : 0;
		desc.Triangles.IndexCount = index_count;
		desc.Triangles.IndexFormat = index_buffer ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_UNKNOWN;
		desc.Triangles.Transform3x4 = 0;
		desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

		//Compute Buffer Size
		UINT scratch_buffer_size{}, result_buffer_size{};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS as_input{};
		as_input.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		as_input.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		as_input.NumDescs = 1u;
		as_input.pGeometryDescs = &desc;
		as_input.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info{};

		graphics_device->GetLatestDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&as_input, &info);

		scratch_buffer_size = _ALIGNMENT_(info.ScratchDataSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		result_buffer_size = _ALIGNMENT_(info.ResultDataMaxSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		graphics_device->CreateBuffer(kDefaultHeapProp, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, scratch_buffer_size,
			D3D12_RESOURCE_STATE_COMMON, scratch_resource_object_.ReleaseAndGetAddressOf());
		graphics_device->CreateBuffer(kDefaultHeapProp, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			result_buffer_size, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			result_resource_object_.ReleaseAndGetAddressOf());

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc{};
		build_desc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		build_desc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		build_desc.Inputs.NumDescs = 1u;
		build_desc.Inputs.pGeometryDescs = &desc;
		build_desc.DestAccelerationStructureData = result_resource_object_->GetGPUVirtualAddress();
		build_desc.ScratchAccelerationStructureData = scratch_resource_object_->GetGPUVirtualAddress();
		build_desc.SourceAccelerationStructureData = 0;
		build_desc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

		graphics_command_list->GetCommandList4()->BuildRaytracingAccelerationStructure(&build_desc,
			0u, nullptr);

		graphics_command_list->SetUavBarrier(result_resource_object_.Get(), D3D12_RESOURCE_BARRIER_FLAG_NONE);
	}

	//BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(const GraphicsDevice* graphics_device,
	//	GraphicsCommandList* graphics_command_list, const VertexBuffer* vertex_buffer, const IndexBuffer* index_buffer)
	//{

	//}
}
