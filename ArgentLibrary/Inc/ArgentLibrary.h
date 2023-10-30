#pragma once

namespace argent
{
	/**
	 * \brief Argent Library初期化
	 * この関数をCallするのは1回だけで良い
	 * 描画、入出力、タイマー等のライブラリサブシステムの初期化をする
	 */
	void Initialize();

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
