#pragma once

#include <string>

namespace argent::graphics
{
	//TODO リソースのタイプ分け　とりあえずenumだけ作った　必要なら作る、いらないなら捨てる
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
		//TODO リソースマネージャを作りユニークIDをもたせること
		explicit GameResource(std::string name):
			name_(std::move(name)) {}

		virtual ~GameResource() = default;

		GameResource(const GameResource&) = delete;
		GameResource(const GameResource&&) = delete;
		GameResource& operator= (const GameResource&) = delete;
		GameResource& operator= (const GameResource&&) = delete;


		/**
		 * \brief ImGuiに表示する関数
		 */
		virtual void OnGui() {}

		const std::string& GetName() const { return name_; }

		//In Progress
		/**
		 * \brief Unique ID取得　idはリソースマネージャから発行され、唯一であることが保証されている
		 * \return Unique ID
		 */
		uint64_t GetUniqueId() const { return unique_id_; }
	private:
		std::string name_;
		uint64_t unique_id_{};
	};
}