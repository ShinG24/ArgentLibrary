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

	//�e���v���[�g��p����Constant Buffer Class
	//�f�B�X�N���v�^�q�[�v��p���邩�ǂ����͑I����
	class ConstantBuffer
	{
	public:

		/**
		 * \brief �R���X�g���N�^
		 * \param graphics_device Graphics Device
		 * \param structure_size �^�T�C�Y�i�P�ʂ̓o�C�g�j 
		 * \param back_buffer_counts �C���X�^���X���@���ʂȎ���Ȃ�����back buffer�̐��ł���
		 * \param cbv_heap optional �f�B�X�N���v�^�e�[�u����p����ꍇ�͕K�v
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
		 * \brief GPU�Ƀf�[�^���R�s�[����
		 * \param data �f�[�^
		 * \param frame_index �t���[���C���f�b�N�X 
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
