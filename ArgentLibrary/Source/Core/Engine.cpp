#include "Core/Engine.h"

#include "Core/SubsystemLocator.h"

#include "Subsystem/Platform/Platform.h"


#include "Subsystem/Graphics/Common/RenderContext.h"
#include "Subsystem/Graphics/GraphicsLibrary.h"
#include "Subsystem/Graphics/RenderingManager.h"


#include "Subsystem/Input/InputManager.h"
#include "Subsystem/Scene/BaseScene.h"
#include "Subsystem/Timer/Timer.h"
#include "Subsystem/Scene/SceneManager.h"


namespace argent
{
	static Engine* engine_instance = nullptr;

	Engine* GetEngine()
	{
		return engine_instance;
	}

	Engine::Engine()
	{
		if(engine_instance) _ASSERT_EXPR(false, L"Engineのインスタンスはすでに生成されています\n"
										  "インスタンスは単一である必要があります");
		engine_instance = this;
		subsystem_locator_ = std::make_unique<SubsystemLocator>();
	}

	void Engine::Initialize(long window_width, long window_height)
	{
		window_width_ = window_width;
		window_height_ = window_height;
		subsystem_locator_->Awake();
	}

	void Engine::Finalize() const
	{
		subsystem_locator_->Shutdown();
	}

	/**
	 * \brief メインループ実行
	 */
	void Engine::Run()
	{
		auto platform = subsystem_locator_->GetSubsystem<platform::Platform>();
		auto graphics = subsystem_locator_->GetSubsystem<graphics::GraphicsLibrary>();
		auto rendering = subsystem_locator_->GetSubsystem<graphics::RenderingManager>();
		auto timer = subsystem_locator_->GetSubsystem<Timer>();
		auto input_manager = subsystem_locator_->GetSubsystem<input::InputManager>();
		auto scene = subsystem_locator_->GetSubsystem<scene::SceneManager>();

		graphics::RenderingManager::SceneConstant scene_data{};

		//Main Loop
		while (!platform->GetRequestShutdown())
		{
			if(!platform->ProcessSystemEventQueue())
			{
				input_manager->Update();
				timer->Tick();

				timer->ShowFrameTime(platform->GetHwnd());

				scene->Update();

				//描画
				auto render_context = graphics->FrameBegin();

				rendering->Render(&render_context);

				graphics->FrameEnd();
			}
		}
	}
}
