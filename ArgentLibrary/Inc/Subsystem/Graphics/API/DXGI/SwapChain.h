#pragma once

#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>


namespace argent::graphics
{
	//TODO �E�B���h�E�����T�C�Y���ꂽ�Ƃ��p�̊֐������
	/**
	 * \brief �X���b�v�`�F�[���̃��b�v�N���X
	 * �v���[���g���߂͂����ł��܂�
	 */
	class SwapChain
	{
	public:

		SwapChain() = default;
		~SwapChain() = default;

		SwapChain(const SwapChain&) = delete;
		SwapChain(const SwapChain&&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&&) = delete;

		/**
		 * \brief �������֐�
		 * \param hwnd Window Handle
		 * \param dxgi_factory Factory
		 * \param command_queue Command Queue
		 * \param num_back_buffers �o�b�N�o�b�t�@��
		 */
		void Awake(HWND hwnd, IDXGIFactory6* dxgi_factory, ID3D12CommandQueue* command_queue, UINT num_back_buffers);

		/**
		 * \brief �o�b�N�o�b�t�@�����ւ���
		 */
		void Present() const;

		/**
		 * \brief �ŏ��Ƀ����_�[�^�[�Q�b�g���쐬����Ƃ��Ɏg���悤
		 * Back Buffer���擾����֐�
		 * \param buffer_index Back Buffer Index
		 * \param pp_resource ID3D12Resource
		 * \return Hresult
		 */
		HRESULT GetBuffer(UINT buffer_index, ID3D12Resource** pp_resource) const;

		/**
		 * \brief ���݂̃o�b�N�o�b�t�@�C���f�b�N�X���擾
		 * \return Current Back Buffer Index
		 */
		UINT GetCurrentBackBufferIndex() const;

		/**
		 * \brief �X���b�v�`�F�[���o�b�t�@�̕�
		 * \return Width
		 */
		UINT64 GetWidth() const { return buffer_width_; }

		/**
		 * \brief �X���b�v�`�F�[���o�b�t�@�̍���
		 * \return Height
		 */
		UINT GetHeight() const { return buffer_height_; }

	private:

		Microsoft::WRL::ComPtr<IDXGISwapChain4> swap_chain_object_;
		UINT64 buffer_width_;
		UINT buffer_height_;
	};
}