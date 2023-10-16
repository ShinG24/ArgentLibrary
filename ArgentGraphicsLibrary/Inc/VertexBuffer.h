#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "GraphicsDevice.h"

namespace argent::graphics
{

	template<typename T>
	class VertexBuffer
	{
	public:
		VertexBuffer(const GraphicsDevice* graphics_device, const T* p_data, UINT vertex_counts, bool allow_update = false);

		~VertexBuffer()
		{
			if(!is_unmapped_) buffer_object_->Unmap(0u, nullptr);	
		}

		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer(const VertexBuffer&&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
		VertexBuffer& operator=(const VertexBuffer&&) = delete;

		D3D12_VERTEX_BUFFER_VIEW GetView() const { return buffer_view_; }
		UINT GetVertexCounts() const { return buffer_view_.SizeInBytes / buffer_view_.StrideInBytes; }

		ID3D12Resource* GetBufferObject() const { return buffer_object_.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer_object_{};
		D3D12_VERTEX_BUFFER_VIEW buffer_view_{};
		T* mapped_data_{};
		bool is_unmapped_;
	};

	template <typename T>
	VertexBuffer<T>::VertexBuffer(const GraphicsDevice* graphics_device, const T* p_data, UINT vertex_counts, bool allow_update)
	{
		if(vertex_counts == 0u) _ASSERT_EXPR(FALSE, L"Vertex Counts need to be more than 1");
		graphics_device->CreateVertexBufferAndView(sizeof(T), vertex_counts, buffer_object_.ReleaseAndGetAddressOf(), buffer_view_);

		buffer_object_->Map(0u, nullptr, reinterpret_cast<void**>(&mapped_data_));

		memcpy(mapped_data_, p_data, sizeof(T) * vertex_counts);

		if(!allow_update) buffer_object_->Unmap(0u, nullptr);
		is_unmapped_ = !allow_update;
	}
}
