#pragma once

#include <d3d12.h>
#include <wrl.h>

namespace argent::graphics::dx12
{
	/**
	 * \brief 	ID3D12GraphicsCommandList��Wrap�N���X
	 * �ʏ�̕`��p�̃R�}���h���X�g�ɉ����A
	 * ���C�g���[�V���O�p�̃I�u�W�F�N�g�������Ă���
	 */
	class GraphicsCommandList
	{
	public:

		GraphicsCommandList() = default;
		~GraphicsCommandList() = default;

		GraphicsCommandList(GraphicsCommandList&) = delete;
		GraphicsCommandList(GraphicsCommandList&&) = delete;
		GraphicsCommandList& operator=(GraphicsCommandList&) = delete;
		GraphicsCommandList& operator=(GraphicsCommandList&&) = delete;

		/**
		 * \brief ������
		 * \param device ID3D12Device 
		 */
		void Awake(ID3D12Device* device);

		/**
		 * \brief �R�}���h���X�g��L����
		 */
		void Activate();

		/**
		 * \brief �R�}���h���X�g�����
		 */
		void Deactivate();

		/**
		 * \brief �R�}���h�A���P�[�^�A�R�}���h���X�g�I�u�W�F�N�g�̃��Z�b�g
		 */
		void Reset();

		/**
		 * \brief �R�}���h���X�g�̖��ߔ��s���X�g�b�v
		 */
		void Close();


		/**
		 * \brief �����_�[�^�[�Q�b�g�r���[���N���A
		 * \param rtv_cpu_handle Cpu Handle
		 * \param clear_color �J���[
		 * \param num_rects 0�ł����ł�
		 * \param p_rects Nullptr�ł����ł�
		 */
		void ClearRtv(D3D12_CPU_DESCRIPTOR_HANDLE rtv_cpu_handle, 
		              float clear_color[4], UINT num_rects = 0, const RECT* p_rects = nullptr) const;

		/**
		 * \brief �f�v�X�X�e���V���r���[���N���A
		 * \param dsv_cpu_handle Cpu Handle
		 * \param clear_flags Depth��Stencil�ǂ������N���A���邩�i���邢�͗����j
		 * \param depth �[�x�N���A�o�����[ ��{�I��1
		 * \param stencil �X�e���V���N���A�o�����[�@��{�I��0
		 * \param num_rects 0�ł����ł�
		 * \param p_rects nullptr�ł����ł�
		 */
		void ClearDsv(D3D12_CPU_DESCRIPTOR_HANDLE dsv_cpu_handle,
		              D3D12_CLEAR_FLAGS clear_flags = D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		              float depth = 1.0, UINT8 stencil = 0, UINT num_rects = 0, 
		              const D3D12_RECT* p_rects = nullptr) const;

		/**
		 * \brief Transition Resource Barrier���Z�b�g
		 * \param p_resource �o���A���Z�b�g���郊�\�[�X�̃|�C���^
		 * \param state_before ���݂̃��\�[�X�X�e�[�g
		 * \param state_after �o���A��̃��\�[�X�X�e�[�g
		 */
		void SetTransitionBarrier(ID3D12Resource* p_resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after) const;

		/**
		 * \brief Uav Resource Barrier���Z�b�g
		 * \param p_resource �o���A���Z�b�g���郊�\�[�X�̃|�C���^
		 * \param flags ���Ԃ�None�ł����Ǝv��
		 */
		void SetUavBarrier(ID3D12Resource* p_resource, D3D12_RESOURCE_BARRIER_FLAGS flags) const;

		/**
		 * \brief �����_�[�^�[�Q�b�g���Z�b�g
		 * \param p_rtv_handles �����_�[�^�[�Q�b�g��Cpu Handle
		 * \param p_dsv_handle �[�x�o�b�t�@��Cpu Handle
		 */
		void SetRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE* p_rtv_handles, const D3D12_CPU_DESCRIPTOR_HANDLE* p_dsv_handle) const;

		/**
		 * \brief �����̃����_�[�^�[�Q�b�g�Z�b�g�p
		 * \param num_render_targets �����_�[�^�[�Q�b�g�̐�
		 * \param p_rtv_handles �����_�[�^�[�Q�b�g�̃n���h���X�^�[�g
		 * \param in_a_row_handle �悭�킩��񂯂ǃ|�C���^���A�����Ă邩�ǂ���
		 * \param p_dsv_handle �[�x�n���h��
		 */
		void SetRenderTargets(UINT num_render_targets,
		                      const D3D12_CPU_DESCRIPTOR_HANDLE* p_rtv_handles, bool in_a_row_handle, const D3D12_CPU_DESCRIPTOR_HANDLE* p_dsv_handle) const;

		/**
		 * \brief View Port���Z�b�g
		 * \param num_viewports �r���[�|�[�g�̐�
		 * \param p_viewport �r���[�|�[�g�̃|�C���^
		 */
		void SetViewports(UINT num_viewports, const D3D12_VIEWPORT* p_viewport) const;

		/**
		 * \brief Rect���Z�b�g
		 * \param num_rects Rect�̐�
		 * \param p_rect Rect�ւ̃|�C���^
		 */
		void SetRects(UINT num_rects, const D3D12_RECT* p_rect) const;

		ID3D12GraphicsCommandList* GetCommandList() const { return command_list_.Get(); }
		ID3D12GraphicsCommandList4* GetCommandList4() const { return command_list_.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> command_list_{};
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator_{};
		bool is_close_{};
		bool is_reset_{};

	};
}