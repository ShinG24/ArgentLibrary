#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Subsystem/Subsystem.h"
#include "Subsystem/Input/Keyboard.h"

namespace argent::scene
{
	class BaseScene;

	class SceneManager final : public Subsystem
	{
	public:

		SceneManager();
		~SceneManager() override = default;

		SceneManager(const SceneManager&) = delete;
		SceneManager(const SceneManager&&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;
		SceneManager& operator=(const SceneManager&&) = delete;

		/**
		 * \brief 初期化処理
		 */
		void Awake() override;

		/**
		 * \brief 終了処理
		 */
		void Shutdown() override;

		/**
		 * \brief 更新処理
		 * シーンの遷移を行う
		 * Current SceneがNullptrではないとき、
		 * Current SceneのUpdateが呼ばれる
		 */
		void Update();

		/**
		 * \brief 描画関数
		 */
		void Render() const;

		/**
		 * \brief Gui上に描画する関数
		 */
		void OnGui() const;

		/**
		 * \brief シーンの登録
		 * \param scene_name シーンの名前
		 * \param scene シーンへのポインタ 所有権が移ることに注意
		 */
		void Register(std::string scene_name, BaseScene* scene);

		/**
		 * \brief 次のシーンをセットする
		 * \param scene_name Next Scene Name
		 */
		void SetNextScene(const std::string& scene_name) { next_scene_name_ = scene_name; }

	private:

		void ChangeScene();

	private:

		BaseScene* current_scene_{};
		std::string next_scene_name_{};

		std::unordered_map<std::string, std::unique_ptr<BaseScene>> scene_array_;

	};
}

