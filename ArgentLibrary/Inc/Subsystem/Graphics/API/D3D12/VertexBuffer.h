#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "GraphicsDevice.h"

namespace argent::graphics::dx12
{
	/**
	 * \brief ���_�o�b�t�@���b�v�N���X
	 * �e���v���[�g���g�p���Ă��Ȃ����߁A�R���X�g���N�^�œn�����ƂɂȂ�
	 * Structure Size�ɒ��Ӂ@
	 */
	class VertexBuffer
	{
	public:

		/**
		 * \brief �R���X�g���N�^
		 * \param graphics_device Graphics Device 
		 * \param p_data �f�[�^�ւ̃|�C���^
		 * \param structure_size �\���̂̃T�C�Y
		 * \param vertex_counts ���_��
		 * \param allow_update �f�[�^�̃A�b�v�f�[�g�����邩�ǂ���
		 */
		VertexBuffer(const GraphicsDevice* graphics_device, const void* p_data, UINT structure_size, UINT vertex_counts, bool allow_update = false);

		/**
		 * \brief �f�X�g���N�^
		 * �R���X�g���N�^��allow_update == true�Ƃ����ꍇ�AUnmap()��Call���Ȃ��̂�
		 * �Ō��Call����
		 */
		~VertexBuffer()
		{
			if(!is_unmapped_) buffer_object_->Unmap(0u, nullptr);	
		}

		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer(const VertexBuffer&&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
		VertexBuffer& operator=(const VertexBuffer&&) = delete;

		/**
		 * \brief Buffer View���擾
		 * \return D3D12_VERTEX_BUFFER_VIEW
		 */
		D3D12_VERTEX_BUFFER_VIEW GetView() const { return buffer_view_; }

		//TODO ���O���������Ȃ�
		/**
		 * \brief ���_�����擾
		 * \return Vertex Counts
		 */
		UINT GetVertexCounts() const { return buffer_view_.SizeInBytes / buffer_view_.StrideInBytes; }

		/**
		 * \brief Buffer Object���擾
		 * \return ID3D12Resource
		 */
		ID3D12Resource* GetBufferObject() const { return buffer_object_.Get(); }

		/**
		 * \brief Buffer��GpuAddress���擾
		 * \return D3D12_GPU_VIRTUAL_ADDRESS
		 */
		D3D12_GPU_VIRTUAL_ADDRESS GetBufferLocation() const { return buffer_object_->GetGPUVirtualAddress(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12Resource> buffer_object_{};
		D3D12_VERTEX_BUFFER_VIEW buffer_view_{};
		void* mapped_data_{};
		bool is_unmapped_;

	};
}
