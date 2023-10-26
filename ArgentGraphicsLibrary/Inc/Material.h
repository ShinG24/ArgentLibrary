#pragma once
#include "GameResource.h"

namespace argent::graphics
{
	class GraphicsContext;
}

namespace argent::graphics
{
	class GameResource;

	class Material : public GameResource
	{
	public:
		Material();

		/**
		 * \brief コンストラクタ でーたはstd::move()で移行させるため所有権がなくなることに注意
		 * \param name Material name
		 */
		Material(std::string& name);
		~Material() override = default;

		/**
		 * \brief テクスチャあるいはコンスタントバッファの初期化関数
		 * マテリアルは可能な限り薄い構造にするため、ここでは何もしないようにする
		 * \param graphics_context GraphicsContext
		 */
		virtual void Awake(const GraphicsContext* graphics_context) = 0;
	private:
		
	};
}
