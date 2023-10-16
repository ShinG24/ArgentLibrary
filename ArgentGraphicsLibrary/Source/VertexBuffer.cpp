#include "../Inc/VertexBuffer.h"

namespace argent::graphics
{
	VertexBuffer::VertexBuffer(const GraphicsDevice* graphics_device, const void* p_data, UINT size_of_structure, UINT vertex_counts, bool allow_update)
	{
		if(vertex_counts == 0u) _ASSERT_EXPR(FALSE, L"Vertex Counts need to be more than 1");
		graphics_device->CreateVertexBufferAndView(size_of_structure, vertex_counts, buffer_object_.ReleaseAndGetAddressOf(), buffer_view_);

		buffer_object_->Map(0u, nullptr, reinterpret_cast<void**>(&mapped_data_));

		memcpy(mapped_data_, p_data, size_of_structure * vertex_counts);

		if(!allow_update) buffer_object_->Unmap(0u, nullptr);
		is_unmapped_ = !allow_update;
	}
}