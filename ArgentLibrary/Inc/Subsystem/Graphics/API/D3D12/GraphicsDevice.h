#pragma once

#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>

namespace argent::graphics::dx12
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
		 * Call this function only once.
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

		void CreateRTV(ID3D12Resource* p_resource, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle) const;

		void CreateVertexBufferAndView(UINT size_of_data_type, UINT num_data, 
			ID3D12Resource** pp_vertex_buffer, D3D12_VERTEX_BUFFER_VIEW& vertex_buffer_view) const;
		void CreateIndexBufferAndView(UINT size_of_data_type, UINT num_data, DXGI_FORMAT format, ID3D12Resource** pp_resource, D3D12_INDEX_BUFFER_VIEW& index_buffer_view) const;

		HRESULT CreateBuffer(D3D12_HEAP_PROPERTIES heap_prop, D3D12_RESOURCE_FLAGS resource_flags,
			UINT size, D3D12_RESOURCE_STATES initial_state,
			ID3D12Resource** pp_resource) const;
		HRESULT CreateResource(D3D12_HEAP_PROPERTIES heap_prop, D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES initial_state, 
			ID3D12Resource** pp_resource) const;
		HRESULT CreateTexture2D(D3D12_HEAP_PROPERTIES heap_prop, D3D12_RESOURCE_FLAGS resource_flags,
			DXGI_FORMAT format, UINT width, UINT height, D3D12_RESOURCE_STATES initial_state, 
			ID3D12Resource** pp_resource) const;

		void CreateCBV(ID3D12Resource* constant_buffer, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle) const;
		void CreateCBV(D3D12_GPU_VIRTUAL_ADDRESS buffer_location, UINT buffer_width, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle) const;

		UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE heap_type) const;

		HRESULT SerializeAndCreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& root_signature_desc,
			ID3D12RootSignature** pp_root_signature, 
			D3D_ROOT_SIGNATURE_VERSION root_signature_version = D3D_ROOT_SIGNATURE_VERSION_1) const;

		void CreateBufferSRV(ID3D12Resource* p_resource, UINT num_elements, 
			UINT structure_byte_stride, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle) const;
		void CreateTexture2DSRV(ID3D12Resource* p_resource, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle) const;

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
