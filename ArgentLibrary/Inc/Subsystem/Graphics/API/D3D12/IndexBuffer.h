#pragma once

#include <d3d12.h>
#include <wrl.h>


namespace argent::graphics::dx12
{
	class GraphicsDevice;

	/**
	 * \brief Index Buffer�N���X
	 * �f�[�^�̃��C�A�E�g��Uint32�Œ�Ȃ̂Œ���
	 */
	class IndexBuffer
	{
	public:

		/**
		 * \brief �R���X�g���N�^
		 * \param graphics_device Graphics Device 
		 * \param p_data Index Data�ւ̃|�C���^
		 * \param index_counts �C���f�b�N�X��
		 */
		IndexBuffer(const GraphicsDevice* graphics_device, const UINT32* p_data, UINT index_counts);

		~IndexBuffer() = default;

		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer(const IndexBuffer&&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;
		IndexBuffer& operator=(const IndexBuffer&&) = delete;

		/**
		 * \brief Buffer View���擾
		 * \return D3D12_INDEX_BUFFER_VIEW
		 */
		D3D12_INDEX_BUFFER_VIEW GetView() const { return buffer_view_; }

		/**
		 * \brief Buffer View�̃|�C���^���擾
		 * \return D3D12_INDEX_BUFFER_VIEW Pointer
		 */
		const D3D12_INDEX_BUFFER_VIEW* GetViewPointer() const { return &buffer_view_; }

		/**
		 * \brief Buffer�̃|�C���^���擾
		 * \return ID3D12Resource
		 */
		ID3D12Resource* GetBufferObject() const { return buffer_object_.Get(); }

		//TODO ���̊֐����O�ւ񂶂��
		/**
		 * \brief �C���f�b�N�X�̐����擾
		 * \return Index Counts
		 */
		UINT GetIndexCounts() const { return index_counts_; }

	private:

		Microsoft::WRL::ComPtr<ID3D12Resource> buffer_object_{};
		D3D12_INDEX_BUFFER_VIEW buffer_view_{};
		UINT index_counts_;
	};
}