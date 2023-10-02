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

		graphics_command_list_.OnAwake(graphics_device_.GetDevice());

		swap_chain_.GetBuffer(render_target[0].ReleaseAndGetAddressOf());
		swap_chain_.GetBuffer(render_target[1].ReleaseAndGetAddressOf());
		swap_chain_.GetBuffer(render_target[2].ReleaseAndGetAddressOf());


		descriptor_[0] = rtv_heap_.PopDescriptor();
		descriptor_[1] = rtv_heap_.PopDescriptor();
		descriptor_[2] = rtv_heap_.PopDescriptor();

		graphics_device_.GetDevice()->CreateRenderTargetView(render_target[0].Get(), 
			nullptr, descriptor_[0].cpu_handle_);
		graphics_device_.GetDevice()->CreateRenderTargetView(render_target[1].Get(), 
			nullptr, descriptor_[1].cpu_handle_);
		graphics_device_.GetDevice()->CreateRenderTargetView(render_target[2].Get(), 
			nullptr, descriptor_[2].cpu_handle_);

		current_back_buffer_index_ = swap_chain_.GetCurrentBackBufferIndex();
	}

	void GraphicsLibrary::Activate()
	{
		//const auto rtv_handle = 
		//graphics_command_list_.SetRenderTarget()
	}

	void GraphicsLibrary::Deactivate()
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
