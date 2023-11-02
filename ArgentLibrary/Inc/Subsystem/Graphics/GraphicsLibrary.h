#pragma once
#include <d3d12.h>
#include <windows.h>

#include <memory>

#include "Subsystem/Subsystem.h"

#include "Common/GraphicsCommon.h"
#include "Common/GraphicsContext.h"

namespace argent::graphics
{
	class SwapChain;
	class DxgiFactory;
	struct RenderContext;
	class ImGuiWrapper;
	class FrameResource;
}

namespace argent::graphics::dx12
{
	class GraphicsDevice;
	class CommandQueue;
	class CommandQueue;
	class GraphicsCommandList;
	class GraphicsCommandList;
	class DescriptorHeap;
	class DescriptorHeap;
	class DescriptorHeap;
	class DescriptorHeap;
	class AccelerationStructureManager;
}


namespace argent::graphics
{
	/**
	 * \brief �`�擝���̃N���X
	 *�`��API�̊Ǘ����s���B�`��J�n��I���A�o�b�N�o�b�t�@�̓���ւ��Ȃ�
	 */
	class GraphicsLibrary final : public Subsystem
	{
	public:

		GraphicsLibrary();

		~GraphicsLibrary() override = default;

		GraphicsLibrary(GraphicsLibrary&) = delete;
		GraphicsLibrary(GraphicsLibrary&&) = delete;
		GraphicsLibrary& operator=(GraphicsLibrary&) = delete;
		GraphicsLibrary& operator=(GraphicsLibrary&&) = delete;

		/**
		 * \brief ����������
		 * ID3D12����n�܂�API�I�u�W�F�N�g�̍쐬
		 */
		void Awake() override;

		/**
		 * \brief �I������
		 * Gpu���`����I����܂őҋ@����
		 */
		void Shutdown() override;

		/**
		 * \brief �`��J�n
		 * Current Index�̃R�}���h���X�g��L��������
		 * \return RenderContext
		 */
		RenderContext FrameBegin();

		/**
		 * \brief �`��I��
		 * �R�}���h�L���[����`�施�߂𔭍s����
		 */
		void FrameEnd();

		/**
		 * \brief �o�b�N�o�b�t�@��SubResource���R�s�[����
		 * \param p_resource ID3D12Resource
		 */
		void CopyToBackBuffer(ID3D12Resource* p_resource) const;

		/**
		 * \brief Graphics Context���擾
		 * \return Graphics Context
		 */
		const GraphicsContext* GetGraphicsContext() const { return &graphics_context_; }

	private:

		/**
		 * \brief �f�o�C�X�ɂ݈̂ˑ����Ă���I�u�W�F�N�g���쐬����
		 */
		void CreateDeviceDependencyObjects();

		/**
		 * \brief �f�o�C�X����API�I�u�W�F�N�g�Ɖ�ʃT�C�Y�Ɉˑ����Ă���I�u�W�F�N�g���쐬����
		 */
		void CreateWindowDependencyObjects();

		/**
		 * \brief �f�o�b�O���C���[��L��������
		 */
		void OnDebugLayer() const;

	private:

		HWND hwnd_;	//Window handle

		std::unique_ptr<DxgiFactory> dxgi_factory_;			//Wraped factory
		std::unique_ptr<SwapChain> swap_chain_;				//Wraped SwapChain object
		std::unique_ptr<dx12::GraphicsDevice> graphics_device_;	//Wraped device object
		std::unique_ptr<dx12::CommandQueue> main_rendering_queue_;	//Wraped Command Queue for the screen 
		std::unique_ptr<dx12::CommandQueue> resource_upload_queue_;	//Wraped Command Queue for resource upload for gpu 
		std::unique_ptr<dx12::GraphicsCommandList> graphics_command_list_[kNumBackBuffers];	//Wraped Command List for rendering
		std::unique_ptr<dx12::GraphicsCommandList> resource_upload_command_list_;	//Wraped Command List for upload resource
		std::unique_ptr<dx12::DescriptorHeap> cbv_srv_uav_heap_;	//Wraped DescriptorHeap for CBV, SRV and UAV.
		std::unique_ptr<dx12::DescriptorHeap> rtv_heap_;
		std::unique_ptr<dx12::DescriptorHeap> dsv_heap_;
		std::unique_ptr<dx12::DescriptorHeap> smp_heap_;

		std::unique_ptr<dx12::AccelerationStructureManager> as_manager_;

		GraphicsContext graphics_context_;

		D3D12_VIEWPORT viewport_;
		D3D12_RECT scissor_rect_{};

		std::unique_ptr<FrameResource> frame_resources_[kNumBackBuffers];

		UINT back_buffer_index_{};

		//For imgui
		std::unique_ptr<ImGuiWrapper> imgui_wrapper_;
	};
}