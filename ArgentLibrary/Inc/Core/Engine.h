#pragma once

#include <memory>

namespace argent
{
	class SubsystemLocator;

	/**
	 * \brief Core Class
	 * すべてのサブシステムを統括し、
	 * サブシステムの初期化、メインループ、終了などの処理を行う
	 */
	class Engine
	{
	public:

		Engine();
		~Engine() = default;

		Engine(const Engine&) = delete;
		Engine(const Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine& operator=(const Engine&&) = delete;

		/**
		 * \brief 初期化処理　
		 * 各サブシステムのインスタンス化、及び初期化
		 */
		void Initialize();

		/**
		 * \brief 終了処理
		 * 各サブシステムの終了処理をCall
		 */
		void Finalize();

		/**
		 * \brief アプリケーションの実行
		 */
		void Run();

	private:

		std::unique_ptr<SubsystemLocator> subsystem_locator_;

	};


	/**
	 * \brief 他クラスからのアクセス用
	 * ライブラリ作成者のみが触れるようにするため、ArgentLibrary.hにはインクルードしないこと
	 * \return Engine Pointer
	 */
	Engine* GetEngine();
}

