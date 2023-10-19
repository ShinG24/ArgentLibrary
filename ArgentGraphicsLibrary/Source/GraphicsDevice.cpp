#include  "../Inc/GraphicsDevice.h"

#include "../Inc/GraphicsCommon.h"


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

		device_->SetName(L"Device");
		latest_device_->SetName(L"Latest Device");
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
		CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, size_of_data_type * num_data, D3D12_RESOURCE_STATE_GENERIC_READ, pp_vertex_buffer);

		vertex_buffer_view.BufferLocation = (*pp_vertex_buffer)->GetGPUVirtualAddress();
		vertex_buffer_view.SizeInBytes = size_of_data_type * num_data;
		vertex_buffer_view.StrideInBytes = size_of_data_type;
	}

	void GraphicsDevice::CreateIndexBufferAndView(UINT size_of_data_type, UINT num_data, DXGI_FORMAT format, ID3D12Resource** pp_resource, D3D12_INDEX_BUFFER_VIEW& index_buffer_view) const
	{
		CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, size_of_data_type * num_data, D3D12_RESOURCE_STATE_GENERIC_READ, pp_resource);

		index_buffer_view.Format = format;
		index_buffer_view.BufferLocation =(*pp_resource)->GetGPUVirtualAddress();
		index_buffer_view.SizeInBytes = size_of_data_type * num_data;
	}

	HRESULT GraphicsDevice::CreateBuffer(D3D12_HEAP_PROPERTIES heap_prop,
										 D3D12_RESOURCE_FLAGS resource_flags, 
										 UINT size, D3D12_RESOURCE_STATES initial_state, 
										 ID3D12Resource** pp_resource) const
	{
		D3D12_RESOURCE_DESC res_desc{};
		res_desc.Alignment = 0u;
		res_desc.DepthOrArraySize = 1u;
		res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		res_desc.Flags = resource_flags;
		res_desc.Format = DXGI_FORMAT_UNKNOWN;
		res_desc.Height = 1u;
		res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		res_desc.MipLevels = 1u;
		res_desc.SampleDesc.Count = 1u;
		res_desc.SampleDesc.Quality = 0u;
		res_desc.Width = size;

		const HRESULT hr = device_->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &res_desc, 
			initial_state, nullptr, IID_PPV_ARGS(pp_resource));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Buffer");
		return hr;
	}

	HRESULT GraphicsDevice::CreateTexture2D(D3D12_HEAP_PROPERTIES heap_prop, 
		D3D12_RESOURCE_FLAGS resource_flags, DXGI_FORMAT format,
		UINT width, UINT height, D3D12_RESOURCE_STATES initial_state, ID3D12Resource** pp_resource) const
	{
		D3D12_RESOURCE_DESC desc{};
		desc.Alignment = 0u;
		desc.DepthOrArraySize = 1u;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Flags = resource_flags;
		desc.Format = format;
		desc.Height = height;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.MipLevels = 1u;
		desc.SampleDesc.Count = 1u;
		desc.SampleDesc.Quality = 0u;
		desc.Width = width;
		const HRESULT hr = device_->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &desc,
			initial_state, nullptr, IID_PPV_ARGS(pp_resource));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Texture2D");
		return hr;
	}

	void GraphicsDevice::CreateCBV(ID3D12Resource* constant_buffer, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle) const
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc{};
		desc.SizeInBytes = static_cast<UINT>(constant_buffer->GetDesc().Width);
		desc.BufferLocation = constant_buffer->GetGPUVirtualAddress();
		device_->CreateConstantBufferView(&desc, cpu_handle);
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
