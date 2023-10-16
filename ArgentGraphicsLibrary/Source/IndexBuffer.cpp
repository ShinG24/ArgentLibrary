#include "../Inc/IndexBuffer.h"

#include "../Inc/GraphicsDevice.h"
namespace argent::graphics
{
	IndexBuffer::IndexBuffer(const GraphicsDevice* graphics_device, const UINT32* p_data, UINT index_counts)
	{
		graphics_device->CreateIndexBufferAndView(sizeof(UINT32), index_counts, 
			DXGI_FORMAT_R32_UINT, buffer_object_.ReleaseAndGetAddressOf(), buffer_view_);

		UINT32* map;
		buffer_object_->Map(0u, nullptr, reinterpret_cast<void**>(&map));

		memcpy(map, p_data, index_counts * sizeof(UINT32));

		buffer_object_->Unmap(0u, nullptr);
	}
}
