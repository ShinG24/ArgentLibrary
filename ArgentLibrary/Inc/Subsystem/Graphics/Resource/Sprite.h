#pragma once


#include <DirectXMath.h>

#include <memory>

#include "Texture.h"
#include "Subsystem/Graphics/Resource/GameResource.h"

namespace argent::graphics
{
	class Texture;

	/**
	 * \brief スプライトレンダラーで使う
	 * テクスチャとUV座標系での原点座標と幅高さを持つ
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

