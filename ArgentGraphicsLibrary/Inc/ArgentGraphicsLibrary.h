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

#include "dxcapi.h"

#include "RasterRenderer.h"
#include "Raytracer.h"

namespace argent::graphics
{
	class GraphicsLibrary
	{
	public:
		static int GetNumBackBuffers() { return kNumBackBuffers;  }

	public:
		GraphicsLibrary() = default;

		~GraphicsLibrary() = default;

		GraphicsLibrary(GraphicsLibrary&) = delete;
		GraphicsLibrary(GraphicsLibrary&&) = delete;
		GraphicsLibrary& operator=(GraphicsLibrary&) = delete;
		GraphicsLibrary& operator=(GraphicsLibrary&&) = delete;

		void Awake(HWND hwnd);
		void Shutdown();

		[[nodiscard]] const GraphicsDevice& GetGraphicsDevice() const { return graphics_device_; }
		[[nodiscard]] const CommandQueue& GetMainRenderingQueue() const { return main_rendering_queue_; }

		void FrameBegin();
		void FrameEnd();
	private:
		void OnRender();

		void CreateDeviceDependencyObjects();

		void OnDebugLayer() const;

	private:
		HWND hwnd_;	//Window handle

		GraphicsDevice graphics_device_;	//Wraped device object
		DxgiFactory dxgi_factory_;			//Wraped factory
		SwapChain swap_chain_;				//Wraped SwapChain object
		CommandQueue main_rendering_queue_;	//Wraped Command Queue for the screen 
		CommandQueue resource_upload_queue_;	//Wraped Command Queue for resource upload for gpu 
		GraphicsCommandList graphics_command_list_[kNumBackBuffers];	//Wraped Command List for rendering
		GraphicsCommandList resource_upload_command_list_;	//Wraped Command List for upload resource
		DescriptorHeap cbv_srv_uav_heap_;	//Wraped DescriptorHeap for CBV, SRV and UAV.
		DescriptorHeap rtv_heap_;
		DescriptorHeap dsv_heap_;
		DescriptorHeap smp_heap_;

		Fence fence_;
		UINT fence_value_ = 0;

		FrameResource frame_resources_[kNumBackBuffers];

		UINT back_buffer_index_{};

		D3D12_VIEWPORT viewport_;
		D3D12_RECT scissor_rect_{};

	private:

		void CreateRaytracingRootSignature();

		RasterRenderer raster_renderer_;
		Raytracer raytracer_;
		bool on_raster_mode_ = false;
	};
}