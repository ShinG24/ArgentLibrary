#include "../Inc/SwapChain.h"

namespace argent::graphics
{
	void SwapChain::Awake(HWND hwnd, IDXGIFactory6* dxgi_factory, ID3D12CommandQueue* command_queue, UINT num_back_buffers)
	{
		HRESULT hr{ S_OK };
		RECT rect{};
		GetWindowRect(hwnd, &rect);

		const UINT window_width = rect.right - rect.left;
		const  UINT window_height = rect.bottom - rect.top;
		buffer_width_ = window_width;
		buffer_height_ = window_height;

		//スワップチェーンの作成
		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1{};
		swap_chain_desc1.Width = window_width;
		swap_chain_desc1.Height = window_height;
		swap_chain_desc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swap_chain_desc1.Stereo = FALSE;
		swap_chain_desc1.SampleDesc.Count = 1;
		swap_chain_desc1.SampleDesc.Quality = 0;
		swap_chain_desc1.BufferUsage = DXGI_USAGE_BACK_BUFFER;
		swap_chain_desc1.BufferCount = num_back_buffers;
		swap_chain_desc1.Scaling = DXGI_SCALING_STRETCH;
		swap_chain_desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swap_chain_desc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swap_chain_desc1.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		Microsoft::WRL::ComPtr<IDXGISwapChain1> tmp_swap_chain;
		hr = dxgi_factory->CreateSwapChainForHwnd(command_queue, hwnd,
			&swap_chain_desc1, nullptr, nullptr,
			tmp_swap_chain.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Swap Chain");

		hr = tmp_swap_chain->QueryInterface(IID_PPV_ARGS(swap_chain_object_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Query Interface SwapChain1 to SwapChain4");
	}

	void SwapChain::Present() const
	{
		const HRESULT hr = swap_chain_object_->Present(0u, 0u);
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Swap BackBuffer");
	}

	HRESULT SwapChain::GetBuffer(UINT buffer_index, ID3D12Resource** pp_resource) const
	{
		const HRESULT hr = swap_chain_object_->GetBuffer(buffer_index, IID_PPV_ARGS(pp_resource));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Get SwapChainBuffer");
		return hr;
	}

	UINT SwapChain::GetCurrentBackBufferIndex() const
	{
		return swap_chain_object_->GetCurrentBackBufferIndex();
	}
}
