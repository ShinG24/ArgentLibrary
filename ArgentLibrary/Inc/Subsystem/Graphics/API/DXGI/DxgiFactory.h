#pragma once

#include <dxgi1_6.h>
#include <wrl.h>


namespace argent::graphics
{
	/**
	 * \brief ファクトリラップクラス　
	 * 別に特別なことはないはず
	 */
	class DxgiFactory
	{
	public:

		DxgiFactory() = default;
		~DxgiFactory() = default;

		DxgiFactory(DxgiFactory&) = delete;
		DxgiFactory(DxgiFactory&&) = delete;
		DxgiFactory operator=(DxgiFactory&) = delete;
		DxgiFactory operator=(DxgiFactory&&) = delete;

		/**
		 * \brief 初期化関数
		 */
		void Awake();

		/**
		 * \brief ファクトリオブジェクトを取得
		 * \return IDXGIFactory6
		 */
		IDXGIFactory6* GetIDxgiFactory() const { return factory_object_.Get(); }

	private:

		Microsoft::WRL::ComPtr<IDXGIFactory6> factory_object_;

	};
}
