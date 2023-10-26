#pragma once

#include <string>

namespace argent::graphics
{
	enum class ResourceState
	{
		Animation,
		Material,
		Mesh,
		Model,
		Texture,
		Sprite,
	};
	//すべてのゲームで使うリソースの基底クラス
	//マテリアル、メッシュ、テクスチャ、アニメーションなどなど
	//ユーザーの目に触れるリソースのことをGameResourceと呼ぶ
	class GameResource
	{
	public:
		GameResource() = default;
		explicit GameResource(std::string name):
			name_(std::move(name)) {}

		virtual ~GameResource() = default;

		GameResource(const GameResource&) = delete;
		GameResource(const GameResource&&) = delete;
		GameResource& operator= (const GameResource&) = delete;
		GameResource& operator= (const GameResource&&) = delete;

		const std::string& GetName() const { return name_; }

		/**
		 * \brief ImGuiに表示する関数
		 */
		virtual void OnGui() {}

	private:
		std::string name_;	
	};
}