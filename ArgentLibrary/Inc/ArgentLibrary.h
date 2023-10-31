#pragma once
#include <string>

namespace argent
{
	/**
	 * \brief Argent Library初期化
	 * この関数をCallするのは1回だけで良い
	 * 描画、入出力、タイマー等のライブラリサブシステムの初期化をする
	 * \param window_width  : ウィンドウの幅
	 * \param window_height : ウィンドウの高さ
	 */
	void Initialize(long window_width, long window_height);

	/**
	 * \brief Argent Library終了処理
	 * この関数はプログラムが終了するときに1度だけ呼ぶ
	 *
	 */
	void Finalize();

	/**
	 * \brief ライブラリメインループ
	 * ゲームループ処理はここで行われる
	 */
	void AppRun();
}

namespace argent::input
{
	bool GetKey();
	bool GetKeyDown();
	bool GetKeyUp();
}

namespace argent::scene
{
	class BaseScene;
}
namespace argent::scene_management
{
	void RegisterScene(std::string scene_name, scene::BaseScene* scene);
	void SetNextScene(std::string scene_name);
}