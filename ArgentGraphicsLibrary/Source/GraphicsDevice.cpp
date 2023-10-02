#include  "../Inc/GraphicsDevice.h"


namespace argent::graphics
{
	void GraphicsDevice::Awake(IDXGIFactory6* idxgi_factory)
	{
		//アダプタの取得
		//TODO アダプタって保持しておく必要はあるのか？？
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

		HRESULT hr{};
		UINT adapter_index{};
		while (true)
		{
			//アダプターの列挙
			//パフォーマンスの高いやつから取ってくる
			hr = idxgi_factory->EnumAdapterByGpuPreference(adapter_index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Enum Gpu Adapter");

			DXGI_ADAPTER_DESC1 adapter_desc;
			adapter->GetDesc1(&adapter_desc);

			// ソフトウェアアダプター
			if (adapter_desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

			// デバイスを生成できるかチェック	shader model 6_6に対応したもの
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

		//フィーチャーレベルを下げながら作成
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

	void GraphicsDevice::CreateRTV(ID3D12Resource* p_resource, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle) const
	{
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = p_resource->GetDesc().Format;
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		device_->CreateRenderTargetView(p_resource, &desc, cpu_handle);
	}

	UINT GraphicsDevice::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE heap_type) const
	{
		return device_->GetDescriptorHandleIncrementSize(heap_type);
	}
}
