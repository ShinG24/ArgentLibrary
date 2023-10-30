#pragma once

#include <d3d12.h>

namespace argent::graphics::dx12
{

	//よく使うヒーププロパティ
	static constexpr D3D12_HEAP_PROPERTIES kDefaultHeapProp{ D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0u, 0u };
	static constexpr D3D12_HEAP_PROPERTIES kUploadHeapProp{ D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0u, 0u };

}