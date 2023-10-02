#pragma once
#include <windows.h>

//TODO インクルードするべき？？？
#include "DxgiFactory.h"
#include "SwapChain.h"
#include "GraphicsDevice.h"

#include "DescriptorHeap.h"

#include "CommandQueue.h"
#include "GraphicsCommandList.h"

namespace argent::graphics
{
	class GraphicsLibrary
	{
	public:
		static constexpr int kNumBackBuffers = 3;
		static int GetNumBackBuffers() { return kNumBackBuffers;  }

	public:
		GraphicsLibrary() = default;

		~GraphicsLibrary() = default;

		void Awake(HWND hwnd);

		[[nodiscard]] const GraphicsDevice& GetGraphicsDevice() const { return graphics_device_; }
		[[nodiscard]] const CommandQueue& GetMainRenderingQueue() const { return main_rendering_queue_; }

		void Activate();
		void Deactivate();
	private:

		void OnDebugLayer() const;

	private:
		HWND hwnd_;

		GraphicsDevice graphics_device_;
		DxgiFactory dxgi_factory_;
		SwapChain swap_chain_;
		CommandQueue main_rendering_queue_;
		GraphicsCommandList graphics_command_list_;
		DescriptorHeap cbv_srv_uav_heap_;
		DescriptorHeap rtv_heap_;
		DescriptorHeap dsv_heap_;
		DescriptorHeap smp_heap_;


		//TODO
		Microsoft::WRL::ComPtr<ID3D12Resource> render_target[kNumBackBuffers];
		Descriptor descriptor_[kNumBackBuffers];
		Microsoft::WRL::ComPtr<ID3D12Resource> depth_buffer[kNumBackBuffers];
		UINT current_back_buffer_index_{};
	};
}
