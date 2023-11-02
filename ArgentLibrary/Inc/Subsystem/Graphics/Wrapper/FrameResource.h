#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "Subsystem/Graphics/API/D3D12/DescriptorHeap.h"

namespace argent::graphics
{
	namespace dx12
	{
		class GraphicsDevice;
		class GraphicsCommandList;
	}

	class SwapChain;
	struct GraphicsContext;

}
namespace argent::graphics
{
	//TODO �����ɃR���X�^���g�o�b�t�@����������??
	/**
	 * \brief �ŏI�I�ȕ`�挋�ʂ��i�[����I�u�W�F�N�g��ێ�����N���X
	 * �����_�[�^�[�Q�b�g�A�[�x�o�b�t�@������
	 */
	class FrameResource
	{
	public:

		FrameResource() = default;
		~FrameResource() = default;

		FrameResource(FrameResource&) = delete;
		FrameResource(FrameResource&&) = delete;
		FrameResource& operator=(FrameResource&) = delete;
		FrameResource& operator=(FrameResource&&) = delete;

		/**
		 * \brief ������ �X���b�v�`�F�[������o�b�N�o�b�t�@���擾����B
		 * �����_�[�^�[�Q�b�g�r���[�A�[�x�X�e���V���r���[����� 
		 * \param graphics_context Graphics Context
		 * \param back_buffer_index Back Buffer Index
		 */
		void Awake(const GraphicsContext* graphics_context, UINT back_buffer_index);
		void Activate(const dx12::GraphicsCommandList* command_list) const;
		void Deactivate(const dx12::GraphicsCommandList* command_list) const;

		ID3D12Resource* GetBackBuffer() const { return resource_object_.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12Resource> resource_object_{};
		Microsoft::WRL::ComPtr<ID3D12Resource> depth_buffer_{};
		dx12::Descriptor rtv_descriptor_{};
		dx12::Descriptor dsv_descriptor_{};
		UINT back_buffer_index_{};
	};
}
