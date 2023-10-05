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


		void Present() const;

		HRESULT GetBuffer(UINT buffer_index, ID3D12Resource** pp_resource) const;
		UINT GetCurrentBackBufferIndex() const;

		UINT64 GetWidth() const { return buffer_width_; }
		UINT GetHeight() const { return buffer_height_; }
	private:
		Microsoft::WRL::ComPtr<IDXGISwapChain4> swap_chain_object_;
		UINT64 buffer_width_;
		UINT buffer_height_;
	};
}