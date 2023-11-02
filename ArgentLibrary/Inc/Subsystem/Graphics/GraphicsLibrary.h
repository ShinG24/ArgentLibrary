#pragma once
#include <windows.h>

#include <memory>

#include "Subsystem/Subsystem.h"

//TODO インクルードするべき？？？

#include "API/D3D12/GraphicsDevice.h"
#include "API/D3D12/CommandQueue.h"
#include "API/D3D12/GraphicsCommandList.h"
#include "API/D3D12/DescriptorHeap.h"

#include "API/D3D12/BottomLevelAccelerationStructure.h"
#include "API/D3D12/TopLevelAccelerationStructure.h"
#include "Wrapper/DXR/AccelerationStructureManager.h"

#include "API/D3D12/ConstantBuffer.h"

#include "API/DXGI/SwapChain.h"
#include "API/DXGI/DxgiFactory.h"

#include "Common/GraphicsCommon.h"
#include "Common/GraphicsContext.h"

#include "Wrapper/ImGuiWrapper.h"
#include "Wrapper/FrameResource.h"


#include "RasterRenderer.h"
#include "Raytracer.h"

namespace argent::graphics
{
	struct RenderContext;
}

#define _USE_RAY_TRACER_ 1

namespace argent::graphics
{


	/**
	 * \brief 描画統括のクラス
	 */
	class GraphicsLibrary final : public Subsystem
	{
	public:
		static int GetNumBackBuffers() { return kNumBackBuffers;  }

	public:
		GraphicsLibrary();

		~GraphicsLibrary() override = default;

		GraphicsLibrary(GraphicsLibrary&) = delete;
		GraphicsLibrary(GraphicsLibrary&&) = delete;
		GraphicsLibrary& operator=(GraphicsLibrary&) = delete;
		GraphicsLibrary& operator=(GraphicsLibrary&&) = delete;

		void Awake() override;
		void Shutdown() override;

		RenderContext FrameBegin();
		void FrameEnd();

		void CopyToBackBuffer(ID3D12Resource* p_resource) const;

		const GraphicsContext* GetGraphicsContext() const { return &graphics_context_; }
	private:

		void InitializeScene();
		void OnRender();

		void CreateDeviceDependencyObjects();
		void CreateWindowDependencyObjects();

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

		FrameResource frame_resources_[kNumBackBuffers];

		UINT back_buffer_index_{};

		D3D12_VIEWPORT viewport_;
		D3D12_RECT scissor_rect_{};

	private:

		RasterRenderer raster_renderer_;

#if _USE_RAY_TRACER_
		/*Raytracer raytracer_;*/
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