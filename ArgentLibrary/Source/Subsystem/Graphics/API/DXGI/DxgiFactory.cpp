#include "Subsystem/Graphics/API/DXGI/DxgiFactory.h"

namespace argent::graphics
{
	void DxgiFactory::Awake()
	{
		UINT factory_creation_flags{};

#ifdef _DEBUG
		factory_creation_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif

		Microsoft::WRL::ComPtr<IDXGIFactory2> tmp_factory;
		HRESULT hr = CreateDXGIFactory2(factory_creation_flags, IID_PPV_ARGS(tmp_factory.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Factory");

		hr = tmp_factory->QueryInterface(IID_PPV_ARGS(factory_object_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Query Interface IDXGIFactory2 to IDXGIFactory6");
	}
	
}
