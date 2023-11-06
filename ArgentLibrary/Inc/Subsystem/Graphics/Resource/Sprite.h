#pragma once


#include <DirectXMath.h>

#include <memory>

#include "Texture.h"
#include "Subsystem/Graphics/Resource/GameResource.h"

namespace argent::graphics
{
	class Texture;

	/**
	 * \brief �X�v���C�g�����_���[�Ŏg��
	 * �e�N�X�`����UV���W�n�ł̌��_���W�ƕ�����������
	 */
	class Sprite final : public GameResource
	{
	public:


		std::shared_ptr<Texture> GetTexture() const { return texture_; }

		void SetUvOrigin(const DirectX::XMFLOAT2& origin) { uv_origin_ = origin; }
		void SetUvSize(const DirectX::XMFLOAT2& size) { uv_size_ = size; }

		DirectX::XMFLOAT2 GetUvOrigin() const { return uv_origin_; }
		DirectX::XMFLOAT2 GetUvSize() const { return uv_size_; }
	private:

		std::shared_ptr<Texture> texture_;
		DirectX::XMFLOAT2 uv_origin_;
		DirectX::XMFLOAT2 uv_size_;
	};
}

