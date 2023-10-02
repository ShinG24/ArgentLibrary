#pragma once

#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>


namespace argent::graphics
{
	class SwapChain
	{
	public:
		SwapChain() = default;
		~SwapChain() = default;

		void Awake(HWND hwnd, IDXGIFactory6* dxgi_factory, ID3D12CommandQueue* command_queue, UINT num_back_buffers);

		HRESULT GetBuffer(ID3D12Resource** pp_resource) const;

		UINT GetCurrentBackBufferIndex() const;
	private:
		Microsoft::WRL::ComPtr<IDXGISwapChain4> swap_chain_object_;
	};
}