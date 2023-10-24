#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "GraphicsCommon.h"

#include "GraphicsDevice.h"

namespace argent::graphics
{
	//Wrap Class of Constant Buffer
	//This class has back_buffer_counts id3d12resource object.
	template<typename T>
	class ConstantBuffer
	{
	public:
		ConstantBuffer(const GraphicsDevice* graphics_device, UINT back_buffer_counts);
		~ConstantBuffer() = default;

		ConstantBuffer(const ConstantBuffer&) = delete;
		ConstantBuffer(const ConstantBuffer&&) = delete;
		ConstantBuffer& operator=(ConstantBuffer&) = delete;
		ConstantBuffer& operator=(ConstantBuffer&&) = delete;

		//void Create(const GraphicsDevice& graphics_device, UINT num_instances);

		D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress(int frame_index) const
		{
			return resource_objects_->GetGPUVirtualAddress() + aligned_data_size_ * frame_index;
		}

		void CopyToGpu(T& data, UINT frame_index)
		{
			memcpy(mapped_data + frame_index * aligned_data_size_, &data, sizeof(T));
		}
	private:
		uint8_t* mapped_data;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_objects_;
		UINT aligned_data_size_;
		UINT num_instances_;
	};

	template <typename T>
	ConstantBuffer<T>::ConstantBuffer(const GraphicsDevice* graphics_device, UINT back_buffer_counts)
	{
		num_instances_ = back_buffer_counts;
		aligned_data_size_ = _ALIGNMENT_(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		graphics_device->CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, 
			aligned_data_size_ * num_instances_, D3D12_RESOURCE_STATE_GENERIC_READ, resource_objects_.ReleaseAndGetAddressOf());

		resource_objects_->Map(0u, nullptr, reinterpret_cast<void**>(&mapped_data));

	}
}
