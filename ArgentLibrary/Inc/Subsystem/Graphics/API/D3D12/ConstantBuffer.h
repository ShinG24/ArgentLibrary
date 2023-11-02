#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <cstdint>
#include <vector>

#include "DescriptorHeap.h"

namespace argent::graphics::dx12
{
	class GraphicsDevice;
	class DescriptorHeap;

	//テンプレートを用いたConstant Buffer Class
	//ディスクリプタヒープを用いるかどうかは選択式
	class ConstantBuffer
	{
	public:

		/**
		 * \brief コンストラクタ
		 * \param graphics_device Graphics Device
		 * \param structure_size 型サイズ（単位はバイト） 
		 * \param back_buffer_counts インスタンス数　特別な事情がない限りback bufferの数でいい
		 * \param cbv_heap optional ディスクリプタテーブルを用いる場合は必要
		 */
		ConstantBuffer(const GraphicsDevice* graphics_device, size_t structure_size, UINT back_buffer_counts,
			DescriptorHeap* cbv_heap = nullptr);
		~ConstantBuffer() = default;

		ConstantBuffer(const ConstantBuffer&) = delete;
		ConstantBuffer(const ConstantBuffer&&) = delete;
		ConstantBuffer& operator=(ConstantBuffer&) = delete;
		ConstantBuffer& operator=(ConstantBuffer&&) = delete;

		D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress(UINT frame_index) const
		{
			return resource_objects_->GetGPUVirtualAddress() + aligned_data_size_ * frame_index;
		}

		const Descriptor& GetDescriptor(UINT frame_index) const
		{
#ifdef _DEBUG
			if (frame_index >= num_instances_) _ASSERT_EXPR(FALSE, L"Frame Index is larger than Num Instances");
#endif
			return descriptors_.at(frame_index);
		}

		/**
		 * \brief GPUにデータをコピーする
		 * \param data データ
		 * \param frame_index フレームインデックス 
		 */
		void CopyToGpu(const void* data, UINT frame_index) const
		{
#ifdef _DEBUG
			if (frame_index >= num_instances_) _ASSERT_EXPR(FALSE, L"Frame Index is larger than Num Instances");
#endif
			memcpy(mapped_data_ + frame_index * aligned_data_size_, data, structure_size_);
		}

	private:

		uint8_t* mapped_data_{};
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_objects_;
		size_t structure_size_;
		size_t aligned_data_size_;
		UINT num_instances_;
		std::vector<Descriptor> descriptors_;
	};
}
