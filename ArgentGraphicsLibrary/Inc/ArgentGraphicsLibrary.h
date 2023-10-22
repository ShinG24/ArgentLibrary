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


#include "FrameResource.h"

#include "RasterRenderer.h"
#include "Raytracer.h"
#include "ImGuiWrapper.h"
#include "ConstantBuffer.h"

#define _USE_RAY_TRACER_ 1

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

		void InitializeScene();
		void OnRender();

		void CreateDeviceDependencyObjects();
		void CreateWindowDependencyObjects();

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


		FrameResource frame_resources_[kNumBackBuffers];

		UINT back_buffer_index_{};

		D3D12_VIEWPORT viewport_;
		D3D12_RECT scissor_rect_{};

	private:

		RasterRenderer raster_renderer_;

#if _USE_RAY_TRACER_
		Raytracer raytracer_;
#endif

		struct SceneConstant
		{
			DirectX::XMFLOAT4X4 view_projection_;
			DirectX::XMFLOAT4X4 inv_view_projection_;
			DirectX::XMFLOAT4 camera_position_;
			DirectX::XMFLOAT4 light_position_;
		};
		ConstantBuffer<SceneConstant> scene_constant_buffer_;

		//Camera
		DirectX::XMFLOAT4 camera_position_{ 0.0, 0.0f, -10.0f, 1.0f };
		DirectX::XMFLOAT3 camera_rotation_{};
		float near_z_ = 0.1f;
		float far_z_ = 1000.0f;
		float fov_angle_ = 60.0f;
		float aspect_ratio_ = 16.0f / 9.0f;
		float rotation_speed_ = 0.003;
		float move_speed_ = 0.5f;

		DirectX::XMFLOAT4 light_position{ 100.0, 100.0f, -100.0f, 1.0f };
		

		bool on_raster_mode_ = false;


	private:
		//For imgui
		Descriptor imgui_font_srv_descriptor_;
		ImGuiWrapper imgui_wrapper_;
	};
}