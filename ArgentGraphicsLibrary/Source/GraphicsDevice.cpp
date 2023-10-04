#include  "../Inc/GraphicsDevice.h"


namespace argent::graphics
{
	void GraphicsDevice::Awake(IDXGIFactory6* idxgi_factory)
	{
		if(device_)
		{
			_ASSERT_EXPR(FALSE, L"Invalied Call of GraphicsDevice::Awake(...) funcion : only be called once");
		}

		//Get Adapter
		//TODO Is it need to hold an adapter??
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

		HRESULT hr{};
		UINT adapter_index{};
		while (true)
		{
			//Enum adater from higth performance to low.
			hr = idxgi_factory->EnumAdapterByGpuPreference(adapter_index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Enum Gpu Adapter");

			DXGI_ADAPTER_DESC1 adapter_desc;
			adapter->GetDesc1(&adapter_desc);

			//software adapter
			if (adapter_desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

			//To Cheack Create Device. Only Support Shader Model 6_6
			hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr);

			if(SUCCEEDED(hr))
			{
				break;
			}
			adapter_index++;
		}

		constexpr  D3D_FEATURE_LEVEL feature_levels[]
		{
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
		};

		//Create Device from high feature level to low level
		for(int i = 0; i < 2; ++i)
		{
			hr = D3D12CreateDevice(adapter.Get(), feature_levels[i], IID_PPV_ARGS(device_.ReleaseAndGetAddressOf()));

			if (SUCCEEDED(hr)) break;
		}

		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create ID3D12Device");

		hr = device_->QueryInterface(IID_PPV_ARGS(latest_device_.ReleaseAndGetAddressOf()));
		if(FAILED(hr))
		{
			latest_device_ = nullptr;
		}
	}

	HRESULT GraphicsDevice::CreateDescriptorHeap(ID3D12DescriptorHeap** pp_descriptor_heap,
		D3D12_DESCRIPTOR_HEAP_FLAGS heap_flags, D3D12_DESCRIPTOR_HEAP_TYPE heap_type, UINT num_descriptors,
		UINT node_mask) const
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Flags = heap_flags;
		desc.NodeMask = node_mask;
		desc.NumDescriptors = num_descriptors;
		desc.Type = heap_type;
		const HRESULT hr = device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(pp_descriptor_heap));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create ID3D12DescriptorHeap");
		return hr;
	}

	HRESULT GraphicsDevice::CreateRenderTarget()
	{
		return E_FAIL;
	}

	HRESULT GraphicsDevice::CreateFence(ID3D12Fence** pp_fence, UINT64 initial_fence_value, D3D12_FENCE_FLAGS fence_flags) const
	{
	 	const HRESULT hr = device_->CreateFence(initial_fence_value, fence_flags, IID_PPV_ARGS(pp_fence));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create ID3D12Fence");
		return hr;
	}

	void GraphicsDevice::CreateRTV(ID3D12Resource* p_resource, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle) const
	{
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = p_resource->GetDesc().Format;
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		device_->CreateRenderTargetView(p_resource, &desc, cpu_handle);
	}

	void GraphicsDevice::CreateVertexBufferAndView(UINT size_of_data_type, UINT num_data, ID3D12Resource** pp_vertex_buffer,
		D3D12_VERTEX_BUFFER_VIEW& vertex_buffer_view) const
	{
		D3D12_HEAP_PROPERTIES heap_prop{};
		heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_prop.CreationNodeMask = 0u;
		heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC res_desc{};
		res_desc.Format = DXGI_FORMAT_UNKNOWN;
		res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		res_desc.Alignment = 0u;
		res_desc.DepthOrArraySize = 1;
		res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		res_desc.Height = 1u;
		res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		res_desc.MipLevels = 1u;
		res_desc.SampleDesc.Count = 1u;
		res_desc.SampleDesc.Quality = 0u;
		res_desc.Width = size_of_data_type * num_data;
		HRESULT hr = device_->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, 
			&res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(pp_vertex_buffer));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create a Buffer");

		vertex_buffer_view.BufferLocation = (*pp_vertex_buffer)->GetGPUVirtualAddress();
		vertex_buffer_view.SizeInBytes = size_of_data_type * num_data;
		vertex_buffer_view.StrideInBytes = size_of_data_type;
	}

	UINT GraphicsDevice::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE heap_type) const
	{
		return device_->GetDescriptorHandleIncrementSize(heap_type);
	}

	HRESULT GraphicsDevice::SerializeAndCreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& root_signature_desc,
		ID3D12RootSignature** pp_root_signature, D3D_ROOT_SIGNATURE_VERSION root_signature_version) const
	{
		Microsoft::WRL::ComPtr<ID3DBlob> blob;
		Microsoft::WRL::ComPtr<ID3DBlob> error;
		HRESULT hr = D3D12SerializeRootSignature(&root_signature_desc, root_signature_version,
			blob.ReleaseAndGetAddressOf(), error.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Serialize the RootSignature");

		hr = device_->CreateRootSignature(0u, blob->GetBufferPointer(),
			blob->GetBufferSize(), IID_PPV_ARGS(pp_root_signature));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create ID3D12RootSignature");
		return hr;
	}

	bool GraphicsDevice::IsDirectXRaytracingSupported() const
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 feature_support_data{};
		HRESULT hr = device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &feature_support_data, sizeof(feature_support_data));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Call ID3D12Device::CheckFeatureSupport()");

		return feature_support_data.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;

	}
}
