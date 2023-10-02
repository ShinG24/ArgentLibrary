#pragma once
#include <windows.h>

//TODO インクルードするべき？？？

#include "GraphicsCommon.h"

#include "DxgiFactory.h"
#include "SwapChain.h"
#include "GraphicsDevice.h"

#include "DescriptorHeap.h"

#include "CommandQueue.h"
#include "GraphicsCommandList.h"

#include "Fence.h"

#include "FrameResource.h"

namespace argent::graphics
{
	class GraphicsLibrary
	{
	public:
		static int GetNumBackBuffers() { return kNumBackBuffers;  }

	public:
		GraphicsLibrary() = default;

		~GraphicsLibrary() = default;

		void Awake(HWND hwnd);

		[[nodiscard]] const GraphicsDevice& GetGraphicsDevice() const { return graphics_device_; }
		[[nodiscard]] const CommandQueue& GetMainRenderingQueue() const { return main_rendering_queue_; }

		void FrameBegin();
		void FrameEnd();
	private:

		void OnDebugLayer() const;

	private:
		HWND hwnd_;

		GraphicsDevice graphics_device_;
		DxgiFactory dxgi_factory_;
		SwapChain swap_chain_;
		CommandQueue main_rendering_queue_;
		GraphicsCommandList graphics_command_list_[kNumBackBuffers];
		DescriptorHeap cbv_srv_uav_heap_;
		DescriptorHeap rtv_heap_;
		DescriptorHeap dsv_heap_;
		DescriptorHeap smp_heap_;

		Fence fence_;

		FrameResource frame_resources_[kNumBackBuffers];

		UINT back_buffer_index_{};
	};
}
