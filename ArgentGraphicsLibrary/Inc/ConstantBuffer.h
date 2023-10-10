#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "GraphicsCommon.h"

#include "GraphicsDevice.h"
#include "DescriptorHeap.h"

namespace argent::graphics
{
	//Wrap Class of Constant Buffer
	//This class has back_buffer_counts id3d12resource object.
	template<typename T>
	class ConstantBuffer
	{
	public:
		ConstantBuffer() = default;
		~ConstantBuffer() = default;

		ConstantBuffer(const ConstantBuffer&) = delete;
		ConstantBuffer(const ConstantBuffer&&) = delete;
		ConstantBuffer& operator=(ConstantBuffer&) = delete;
		ConstantBuffer& operator=(ConstantBuffer&&) = delete;

		void Awake(const GraphicsDevice& graphics_device, DescriptorHeap& cbv_descriptor_heap);
		void Update(const T& data, int frame_index);

		T GetData(UINT frame_index) const { return *data_[frame_index]; }

		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(int frame_index) const { return descriptors_[frame_index].cpu_handle_; }
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(int frame_index) const { return descriptors_[frame_index].gpu_handle_; }

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_objects_[kNumBackBuffers];
		Descriptor descriptors_[kNumBackBuffers];
		T* data_[kNumBackBuffers];
	};


	template <typename T>
	void ConstantBuffer<T>::Awake(const GraphicsDevice& graphics_device, DescriptorHeap& cbv_descriptor_heap)
	{
		for(int i = 0; i < kNumBackBuffers; ++i)
		{
			//Create Buffer
			graphics_device.CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, 
				_ALIGNMENT_(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), 
				D3D12_RESOURCE_STATE_GENERIC_READ, resource_objects_[i].ReleaseAndGetAddressOf());

			//Get Descriptor
			descriptors_[i] = cbv_descriptor_heap.PopDescriptor();

			//Create View
			graphics_device.CreateCBV(resource_objects_[i].Get(), descriptors_[i].cpu_handle_);

			//For reduction of the overhead calling ::Map() function,
			//do not unmap until release this object.
			resource_objects_[i]->Map(0u, nullptr, reinterpret_cast<void**>(&data_[i]));
		}
	}

	template <typename T>
	void ConstantBuffer<T>::Update(const T& data, int frame_index)
	{
		*(data_[frame_index]) = data;
	}
}
