#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "GraphicsDevice.h"

namespace argent::graphics
{
	class VertexBuffer
	{
	public:
		VertexBuffer(const GraphicsDevice* graphics_device, const void* p_data, UINT size_of_structure, UINT vertex_counts, bool allow_update = false);

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
		void* mapped_data_{};
		bool is_unmapped_;
	};
}
