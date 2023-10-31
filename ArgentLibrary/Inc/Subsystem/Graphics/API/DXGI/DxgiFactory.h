#pragma once

#include <dxgi1_6.h>
#include <wrl.h>


namespace argent::graphics
{
	class DxgiFactory
	{
	public:
		DxgiFactory() = default;
		~DxgiFactory() = default;
		DxgiFactory(DxgiFactory&) = delete;
		DxgiFactory(DxgiFactory&&) = delete;
		DxgiFactory operator=(DxgiFactory&) = delete;
		DxgiFactory operator=(DxgiFactory&&) = delete;

		void Awake();

		IDXGIFactory6* GetIDxgiFactory() const { return factory_object_.Get(); }
	private:
		Microsoft::WRL::ComPtr<IDXGIFactory6> factory_object_;
	};
}
