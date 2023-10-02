#include "../Inc/ArgentGraphicsLibrary.h"

#include <windows.h>

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

		//swap_chain_.GetBuffer(0u, render_target[0].ReleaseAndGetAddressOf());
		//swap_chain_.GetBuffer(1u, render_target[1].ReleaseAndGetAddressOf());
		//swap_chain_.GetBuffer(2u, render_target[2].ReleaseAndGetAddressOf());

		//descriptor_[0] = rtv_heap_.PopDescriptor();
		//descriptor_[1] = rtv_heap_.PopDescriptor();
		//descriptor_[2] = rtv_heap_.PopDescriptor();

		//graphics_device_.GetDevice()->CreateRenderTargetView(render_target[0].Get(), 
		//	nullptr, descriptor_[0].cpu_handle_);
		//graphics_device_.GetDevice()->CreateRenderTargetView(render_target[1].Get(), 
		//	nullptr, descriptor_[1].cpu_handle_);
		//graphics_device_.GetDevice()->CreateRenderTargetView(render_target[2].Get(), 
		//	nullptr, descriptor_[2].cpu_handle_);

		fence_.Awake(graphics_device_);

		back_buffer_index_ = swap_chain_.GetCurrentBackBufferIndex();
	}

	void GraphicsLibrary::FrameBegin()
	{
		auto& command_list = graphics_command_list_[back_buffer_index_];
		command_list.Activate();

		frame_resources_[back_buffer_index_].Activate(command_list);

		//command_list.SetTransitionBarrier(render_target[back_buffer_index_].Get(), 
		//	D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		//float clear_color[4]{ 1, 1, 0, 1 };
		//command_list.ClearRtv(descriptor_[back_buffer_index_].cpu_handle_, 
		//	clear_color);

		//const auto rtv_handle = descriptor_[back_buffer_index_].cpu_handle_;
		//command_list.SetRenderTarget(&rtv_handle, nullptr);
	}

	void GraphicsLibrary::FrameEnd()
	{
		auto& command_list = graphics_command_list_[back_buffer_index_];

		//command_list.SetTransitionBarrier(render_target[back_buffer_index_].Get(), 
		//	 D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

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
