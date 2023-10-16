#pragma once

#include <d3d12.h>
#include <wrl.h>


namespace argent::graphics
{
	class GraphicsDevice;
	class GraphicsCommandList;

	//class VertexBuffer;
	//class IndexBuffer;

	class BottomLevelAccelerationStructure
	{
	public:
		BottomLevelAccelerationStructure(const GraphicsDevice* graphics_device, GraphicsCommandList* graphics_command_list,
			ID3D12Resource* vertex_buffer, UINT vertex_count, UINT size_of_vertex_structure,
			ID3D12Resource* index_buffer, UINT index_count);

		//BottomLevelAccelerationStructure(const GraphicsDevice* graphics_device, GraphicsCommandList* graphics_command_list, 
		//	const VertexBuffer* vertex_buffer, const IndexBuffer* index_buffer);
		~BottomLevelAccelerationStructure() = default;

		BottomLevelAccelerationStructure(const BottomLevelAccelerationStructure&) = delete;
		BottomLevelAccelerationStructure(const BottomLevelAccelerationStructure&&) = delete;
		BottomLevelAccelerationStructure& operator=(const BottomLevelAccelerationStructure&&) = delete;
		BottomLevelAccelerationStructure& operator=(const BottomLevelAccelerationStructure&) = delete;

		ID3D12Resource* GetResultBuffer() const { return result_resource_object_.Get(); }
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> scratch_resource_object_;
		Microsoft::WRL::ComPtr<ID3D12Resource> result_resource_object_;
		Microsoft::WRL::ComPtr<ID3D12Resource> instance_resource_object_; //Use only top level
	};
}