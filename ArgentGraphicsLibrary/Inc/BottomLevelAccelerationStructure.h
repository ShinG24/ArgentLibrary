#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <vector>

namespace argent::graphics
{
	class GraphicsDevice;
	class GraphicsCommandList;

	class VertexBuffer;
	class IndexBuffer;

	/**
	 * \brief Under Development
	 * vertex_buffer_vec_.size() need to the same value index_buffer_vec_.size()
	 */
	struct BLASBuildDesc
	{
		std::vector<VertexBuffer*> vertex_buffer_vec_;
		std::vector<IndexBuffer*> index_buffer_vec_;
	};

	class BottomLevelAccelerationStructure
	{
	public:
		BottomLevelAccelerationStructure(const GraphicsDevice* graphics_device, const GraphicsCommandList* graphics_command_list, 
			const VertexBuffer* vertex_buffer, const IndexBuffer* index_buffer = nullptr);

		BottomLevelAccelerationStructure(const GraphicsDevice* graphics_device, const GraphicsCommandList* graphics_command_list, 
			const BLASBuildDesc* build_desc);

		~BottomLevelAccelerationStructure() = default;

		BottomLevelAccelerationStructure(const BottomLevelAccelerationStructure&) = delete;
		BottomLevelAccelerationStructure(const BottomLevelAccelerationStructure&&) = delete;
		BottomLevelAccelerationStructure& operator=(const BottomLevelAccelerationStructure&&) = delete;
		BottomLevelAccelerationStructure& operator=(const BottomLevelAccelerationStructure&) = delete;

		ID3D12Resource* GetResultBuffer() const { return result_buffer_object_.Get(); }
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> scratch_buffer_object_;
		Microsoft::WRL::ComPtr<ID3D12Resource> result_buffer_object_;

		UINT scratch_buffer_size_;
		UINT result_buffer_size_;
	};
}