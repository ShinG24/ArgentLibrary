#pragma once
#include <windows.h>

//TODO �C���N���[�h����ׂ��H�H�H

#include "API/D3D12/GraphicsDevice.h"
#include "API/D3D12/CommandQueue.h"
#include "API/D3D12/GraphicsCommandList.h"
#include "API/D3D12/DescriptorHeap.h"
#include "API/D3D12/ConstantBuffer.h"

#include "API/DXGI/SwapChain.h"
#include "API/DXGI/DxgiFactory.h"

#include "Common/GraphicsCommon.h"
#include "Common/GraphicsContext.h"

#include "Wrapper/ImGuiWrapper.h"
#include "Wrapper/FrameResource.h"


#include "RasterRenderer.h"
#include "Raytracer.h"


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

		[[nodiscard]] const dx12::GraphicsDevice& GetGraphicsDevice() const { return graphics_device_; }
		[[nodiscard]] const dx12::CommandQueue& GetMainRenderingQueue() const { return main_rendering_queue_; }

		void FrameBegin();
		void FrameEnd();
	private:

		void InitializeScene();
		void OnRender();

		void CreateDeviceDependencyObjects();
		void CreateWindowDependencyObjects();

		void OnDebugLayer() const;

		const GraphicsContext* GetGraphicsContext() const { return &graphics_context_; }

	private:
		HWND hwnd_;	//Window handle

		dx12::GraphicsDevice graphics_device_;	//Wraped device object
		DxgiFactory dxgi_factory_;			//Wraped factory
		SwapChain swap_chain_;				//Wraped SwapChain object
		dx12::CommandQueue main_rendering_queue_;	//Wraped Command Queue for the screen 
		dx12::CommandQueue resource_upload_queue_;	//Wraped Command Queue for resource upload for gpu 
		dx12::GraphicsCommandList graphics_command_list_[kNumBackBuffers];	//Wraped Command List for rendering
		dx12::GraphicsCommandList resource_upload_command_list_;	//Wraped Command List for upload resource
		dx12::DescriptorHeap cbv_srv_uav_heap_;	//Wraped DescriptorHeap for CBV, SRV and UAV.
		dx12::DescriptorHeap rtv_heap_;
		dx12::DescriptorHeap dsv_heap_;
		dx12::DescriptorHeap smp_heap_;


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

		std::unique_ptr<dx12::ConstantBuffer> scene_constant_buffer_;

		//Camera
		DirectX::XMFLOAT4 camera_position_{ 0.0, 100.0f, -100.0f, 1.0f };
		DirectX::XMFLOAT3 camera_rotation_{ 0.36f, 0.0f, 0.0f};
		float near_z_ = 0.1f;
		float far_z_ = 1000.0f;
		float fov_angle_ = 60.0f;
		float aspect_ratio_ = 16.0f / 9.0f;
		float rotation_speed_ = 0.003f;
		float move_speed_ = 0.5f;

		DirectX::XMFLOAT4 light_position{ 1000.0, 1000.0f, -1000.0f, 1.0f };
		

		bool on_raster_mode_ = false;

		GraphicsContext graphics_context_;

	private:
		//For imgui
		dx12::Descriptor imgui_font_srv_descriptor_;
		ImGuiWrapper imgui_wrapper_;
	};
}