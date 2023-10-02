#pragma once

#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>

namespace argent::graphics
{
	class GraphicsDevice
	{
	public:
		GraphicsDevice():
			latest_device_(nullptr)
		,	device_(nullptr)
		{}

		~GraphicsDevice() = default;

		GraphicsDevice(GraphicsDevice&) = delete;
		GraphicsDevice(GraphicsDevice&&) = delete;
		GraphicsDevice& operator=(GraphicsDevice&) = delete;
		GraphicsDevice& operator=(GraphicsDevice&&) = delete;

		void Awake(IDXGIFactory6* idxgi_factory);

		ID3D12Device* GetDevice() const { return device_.Get(); }
		ID3D12Device8* GetLatestDevice() const { return latest_device_.Get(); }

		HRESULT CreateDescriptorHeap(ID3D12DescriptorHeap** pp_descriptor_heap,
		D3D12_DESCRIPTOR_HEAP_FLAGS heap_flags, D3D12_DESCRIPTOR_HEAP_TYPE heap_type, UINT num_descriptors,
		UINT node_mask = 0) const;

		//TODO
		HRESULT CreateRenderTarget();
		HRESULT CreateFence(ID3D12Fence** pp_fence, UINT64 initial_fence_value = 0, D3D12_FENCE_FLAGS fence_flags = D3D12_FENCE_FLAG_NONE) const;

		void CreateRTV(ID3D12Resource* p_resource, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle) const;




		UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE heap_type) const;

	private:
		Microsoft::WRL::ComPtr<ID3D12Device8> latest_device_;
		Microsoft::WRL::ComPtr<ID3D12Device> device_;
	};
}