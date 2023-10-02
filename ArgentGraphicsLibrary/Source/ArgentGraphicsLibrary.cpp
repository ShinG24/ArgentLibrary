#include "../Inc/ArgentGraphicsLibrary.h"

#include <windows.h>


#include "../Inc/ShaderCompiler.h"

#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3D12.lib")

namespace argent::graphics
{
	void GraphicsLibrary::Awake(HWND hwnd)
	{
		hwnd_ = hwnd;

#ifdef _DEBUG
		OnDebugLayer();
#endif

		dxgi_factory_.Awake();
		graphics_device_.Awake(dxgi_factory_.GetIDxgiFactory());
		main_rendering_queue_.Awake(graphics_device_.GetLatestDevice(), L"Main Rendering Queue");
		swap_chain_.Awake(hwnd, dxgi_factory_.GetIDxgiFactory(), main_rendering_queue_.GetCommandQueue(), kNumBackBuffers);

		cbv_srv_uav_heap_.Awake(graphics_device_, DescriptorHeap::HeapType::CbvSrvUav, 10000);
		rtv_heap_.Awake(graphics_device_, DescriptorHeap::HeapType::Rtv, 100);
		dsv_heap_.Awake(graphics_device_, DescriptorHeap::HeapType::Rtv, 100);
		smp_heap_.Awake(graphics_device_, DescriptorHeap::HeapType::Smp, 50);

		graphics_command_list_[0].OnAwake(graphics_device_.GetDevice());
		graphics_command_list_[1].OnAwake(graphics_device_.GetDevice());
		graphics_command_list_[2].OnAwake(graphics_device_.GetDevice());

		for(int i = 0; i < kNumBackBuffers; ++i)
		{
			frame_resources_[i].Awake(graphics_device_, swap_chain_, i, rtv_heap_.PopDescriptor(), dsv_heap_.PopDescriptor());
		}

		fence_.Awake(graphics_device_);

		back_buffer_index_ = swap_chain_.GetCurrentBackBufferIndex();


		ShaderCompiler compiler;
		compiler.Compile(L"./Assets/Shader/VertexShader.hlsl", L"vs_6_6", vertex_shader_.ReleaseAndGetAddressOf());
		compiler.Compile(L"./Assets/Shader/PixelShader.hlsl", L"ps_6_6", pixel_shader_.ReleaseAndGetAddressOf());

	}

	void GraphicsLibrary::Shutdown()
	{
		fence_.WaitForGpu(back_buffer_index_);
	}

	void GraphicsLibrary::FrameBegin()
	{
		auto& command_list = graphics_command_list_[back_buffer_index_];
		command_list.Activate();

		frame_resources_[back_buffer_index_].Activate(command_list);

		OnRender();
	}

	void GraphicsLibrary::FrameEnd()
	{
		auto& command_list = graphics_command_list_[back_buffer_index_];

		frame_resources_[back_buffer_index_].Deactivate(command_list);

		command_list.Deactivate();

		ID3D12CommandList* command_lists[]
		{
			command_list.GetCommandList()
		};
		main_rendering_queue_.Execute(1u, command_lists);
		main_rendering_queue_.Signal(fence_);

		swap_chain_.Present();

		back_buffer_index_ = swap_chain_.GetCurrentBackBufferIndex();

		fence_.WaitForGpu(back_buffer_index_);
	}

	void GraphicsLibrary::OnRender()
	{

	}

	void GraphicsLibrary::OnDebugLayer() const
	{
		HRESULT hr{ S_OK };
		ID3D12Debug* debugLayer = nullptr;
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Valied D3d12 Debug Layer");

		debugLayer->EnableDebugLayer();
		debugLayer->Release();
	}
}
