#include "Subsystem/Graphics/API/D3D12/VertexBuffer.h"

namespace argent::graphics::dx12
{
	VertexBuffer::VertexBuffer(const GraphicsDevice* graphics_device, const void* p_data, size_t structure_size,
		size_t vertex_counts, bool allow_update)
	{
		if(vertex_counts == 0u) _ASSERT_EXPR(FALSE, L"Vertex Counts need to be more than 1");
		graphics_device->CreateVertexBufferAndView(static_cast<UINT>(structure_size),
			static_cast<UINT>(vertex_counts), buffer_object_.ReleaseAndGetAddressOf(), buffer_view_);

		buffer_object_->Map(0u, nullptr, reinterpret_cast<void**>(&mapped_data_));

		memcpy(mapped_data_, p_data, structure_size * vertex_counts);

		if(!allow_update) buffer_object_->Unmap(0u, nullptr);
		is_unmapped_ = !allow_update;
	}
}
