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
	 * \brief 描画統括のクラス
	 *描画APIの管理を行う。描画開始や終了、バックバッファの入れ替えなど
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
		 * \brief 初期化処理
		 * ID3D12から始まるAPIオブジェクトの作成
		 */
		void Awake() override;

		/**
		 * \brief 終了処理
		 * Gpuが描画を終えるまで待機する
		 */
		void Shutdown() override;

		/**
		 * \brief 描画開始
		 * Current Indexのコマンドリストを有効化する
		 * \return RenderContext
		 */
		RenderContext FrameBegin();

		/**
		 * \brief 描画終了
		 * コマンドキューから描画命令を発行する
		 */
		void FrameEnd();

		/**
		 * \brief バックバッファにSubResourceをコピーする
		 * \param p_resource ID3D12Resource
		 */
		void CopyToBackBuffer(ID3D12Resource* p_resource) const;

		/**
		 * \brief Graphics Contextを取得
		 * \return Graphics Context
		 */
		const GraphicsContext* GetGraphicsContext() const { return &graphics_context_; }

	private:

		/**
		 * \brief デバイスにのみ依存しているオブジェクトを作成する
		 */
		void CreateDeviceDependencyObjects();

		/**
		 * \brief デバイス等のAPIオブジェクトと画面サイズに依存しているオブジェクトを作成する
		 */
		void CreateWindowDependencyObjects();

		/**
		 * \brief デバッグレイヤーを有効化する
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