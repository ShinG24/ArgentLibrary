#pragma once

#include <d3d12.h>
#include <wrl.h>


namespace argent::graphics
{
	class GraphicsDevice;

	class IndexBuffer
	{
	public:
		IndexBuffer(const GraphicsDevice* graphics_device, const UINT32* p_data, UINT index_counts);
		~IndexBuffer() = default;

		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer(const IndexBuffer&&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;
		IndexBuffer& operator=(const IndexBuffer&&) = delete;

		D3D12_INDEX_BUFFER_VIEW GetBufferView() const { return buffer_view_; }
		ID3D12Resource* GetBufferObject() const { return buffer_object_.Get(); }
		UINT GetIndexCounts() const { return buffer_view_.SizeInBytes / sizeof(UINT32); }
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer_object_{};
		D3D12_INDEX_BUFFER_VIEW buffer_view_{};
	};
}