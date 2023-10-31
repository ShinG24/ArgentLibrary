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
		void Initialize(long window_width, long window_height);

		/**
		 * \brief 終了処理
		 * 各サブシステムの終了処理をCall
		 */
		void Finalize() const;

		/**
		 * \brief アプリケーションの実行
		 */
		void Run();

		SubsystemLocator* GetSubsystemLocator() const { return subsystem_locator_.get(); }

		long GetWindowWidth() const { return window_width_; }
		long GetWindowHeight() const { return window_height_; }

	private:

		std::unique_ptr<SubsystemLocator> subsystem_locator_;
		long window_width_;
		long window_height_;

	};


	/**
	 * \brief 他クラスからのアクセス用
	 * ライブラリ作成者のみが触れるようにするため、ArgentLibrary.hにはインクルードしないこと
	 * \return Engine Pointer
	 */
	Engine* GetEngine();

}

