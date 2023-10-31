#include "Subsystem/Graphics/API/D3D12/ConstantBuffer.h"

#include "Subsystem/Graphics/API/D3D12/GraphicsDevice.h"
#include "Subsystem/Graphics/API/D3D12/DescriptorHeap.h"
#include "Subsystem/Graphics/API/D3D12/D3D12Common.h"

#include "Subsystem/Graphics/Common/GraphicsCommon.h"


namespace argent::graphics::dx12
{
	ConstantBuffer::ConstantBuffer(const GraphicsDevice* graphics_device, size_t structure_size, UINT back_buffer_counts,
		DescriptorHeap* cbv_heap) :
		structure_size_(structure_size)
		, aligned_data_size_(_ALIGNMENT_(structure_size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
		, num_instances_(back_buffer_counts)
	{
		graphics_device->CreateBuffer(dx12::kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE,
			aligned_data_size_ * num_instances_, D3D12_RESOURCE_STATE_GENERIC_READ, 
			resource_objects_.ReleaseAndGetAddressOf());
		resource_objects_->Map(0u, nullptr, reinterpret_cast<void**>(&mapped_data_));


		//DescriptorTable‚ğg—p‚·‚éê‡
		if(cbv_heap)
		{
			descriptors_.resize(num_instances_);
			for(size_t i = 0; i < descriptors_.size(); ++i)
			{
				descriptors_.at(i) = cbv_heap->PopDescriptor();
				graphics_device->CreateCBV(resource_objects_->GetGPUVirtualAddress() + aligned_data_size_* i,
					static_cast<UINT>(aligned_data_size_), descriptors_.at(i).cpu_handle_);
			}
		}
	}
}
