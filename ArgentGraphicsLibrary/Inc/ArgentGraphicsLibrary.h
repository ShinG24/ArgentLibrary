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
		GraphicsCommandList graphics_command_list_[kNumBackBuffers];	//Wraped Command List for rendering
		DescriptorHeap cbv_srv_uav_heap_;	//Wraped DescriptorHeap for CBV, SRV and UAV.
		DescriptorHeap rtv_heap_;
		DescriptorHeap dsv_heap_;
		DescriptorHeap smp_heap_;

		Fence fence_;

		FrameResource frame_resources_[kNumBackBuffers];

		UINT back_buffer_index_{};

		D3D12_VIEWPORT viewport_;
		D3D12_RECT scissor_rect_{};



	private:
		void CreateVertexBuffer();
		void CreateRootSignature();
		void CreatePipelineState();

		//TODO
		struct float2
		{
			float x_;
			float y_;
		};
		struct Vertex
		{
			float2 position_;
			float2 texcoord_;
		};
		Microsoft::WRL::ComPtr<IDxcBlob> vertex_shader_;
		Microsoft::WRL::ComPtr<IDxcBlob> pixel_shader_;
		Microsoft::WRL::ComPtr<ID3D12Resource> vertex_buffer_;
		D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state_;
	};
}