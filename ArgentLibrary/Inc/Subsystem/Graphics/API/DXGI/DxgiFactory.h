#pragma once

#include <dxgi1_6.h>
#include <wrl.h>


namespace argent::graphics
{
	/**
	 * \brief �t�@�N�g�����b�v�N���X�@
	 * �ʂɓ��ʂȂ��Ƃ͂Ȃ��͂�
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
		 * \brief �������֐�
		 */
		void Awake();

		/**
		 * \brief �t�@�N�g���I�u�W�F�N�g���擾
		 * \return IDXGIFactory6
		 */
		IDXGIFactory6* GetIDxgiFactory() const { return factory_object_.Get(); }

	private:

		Microsoft::WRL::ComPtr<IDXGIFactory6> factory_object_;

	};
}
