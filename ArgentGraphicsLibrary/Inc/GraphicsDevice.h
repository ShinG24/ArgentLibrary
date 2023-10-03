#pragma once

#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>

namespace argent::graphics
{
	//Wrap Class of ID3D12Device.
	//Only be used like a DirectX12 API Device.
	//Wrap Major Function
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

		/**
		 * \brief Create ID3D12Device Object.
		 * Call this funcion only once.
		 * \param idxgi_factory a pointer to IDXGIFactory6 object
		 */
		void Awake(IDXGIFactory6* idxgi_factory);

		ID3D12Device* GetDevice() const { return device_.Get(); }
		ID3D12Device8* GetLatestDevice() const { return latest_device_.Get(); }

		/**
		 * \brief Wrap function of ID3D12Device::CreateDescriptorHeap
		 * \param pp_descriptor_heap : double pointer to ID3D12DescriptorHeap Object
		 * \param heap_flags : shader visible or not
		 * \param heap_type : The usage of this heap. CBV, SRV, UAV : RTV : DSV : Sampler
		 * \param num_descriptors : Max Descriptor Counts.
		 * \param node_mask : be 0 if you use single GPU 
		 * \return result
		 */
		HRESULT CreateDescriptorHeap(ID3D12DescriptorHeap** pp_descriptor_heap,
		                             D3D12_DESCRIPTOR_HEAP_FLAGS heap_flags, D3D12_DESCRIPTOR_HEAP_TYPE heap_type, UINT num_descriptors,
		                             UINT node_mask = 0) const;

		//TODO
		HRESULT CreateRenderTarget();
		HRESULT CreateFence(ID3D12Fence** pp_fence, UINT64 initial_fence_value = 0, D3D12_FENCE_FLAGS fence_flags = D3D12_FENCE_FLAG_NONE) const;

		void CreateRTV(ID3D12Resource* p_resource, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle) const;

		UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE heap_type) const;

		/**
		 * \brief Check is This computer or Gpu supported DXR.
		 * \return bool 
		 */
		bool IsDirectXRaytracingSupported() const;
	private:
		Microsoft::WRL::ComPtr<ID3D12Device8> latest_device_;
		Microsoft::WRL::ComPtr<ID3D12Device> device_;
	};
}