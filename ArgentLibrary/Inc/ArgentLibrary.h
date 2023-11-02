#pragma once
#include <string>

#include "Component/Component.h"
#include "Component/Transform.h"
#include "Component/Camera.h"
#include "Component/Light.h"

#include "Subsystem/Input/Keyboard.h"
#include "Subsystem/Input/Mouse.h"

#include "Subsystem/Scene/BaseScene.h"

/**
 * \brief ライブラリのCore機能まとめ
 */
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

/**
 * \brief キーボード、マウス、コントローラ等の入力について
 */
namespace argent::input
{
	bool GetKey(KeyCode key);
	bool GetKeyDown(KeyCode key);
	bool GetKeyUp(KeyCode key);
	bool GetButton(MouseButton button);

	float GetMouseMoveX();
	float GetMouseMoveY();
	DirectX::XMFLOAT2 GetMouseMove();
}

/**
 * \brief シーン管理について
 */
namespace argent::scene_management
{
	/**
	 * \brief シーンの登録
	 * \param scene_name シーンネーム
	 * \param scene シーンへのポインタ
	 */
	void RegisterScene(std::string scene_name, scene::BaseScene* scene);

	/**
	 * \brief シーンをセット
	 * \param scene_name シーンネーム
	 */
	void SetNextScene(std::string scene_name);
}
