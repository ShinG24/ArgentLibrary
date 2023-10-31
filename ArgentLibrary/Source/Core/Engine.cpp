#include "Core/Engine.h"

#include "Core/SubsystemLocator.h"

#include "Subsystem/Platform/Platform.h"
#include "Subsystem/Graphics/GraphicsLibrary.h"
#include "Subsystem/Input/InputManager.h"
#include "Subsystem/Timer/Timer.h"

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

	void Engine::Initialize(long window_width, long window_height) const
	{
		subsystem_locator_->Awake();
	}

	void Engine::Finalize() const
	{
		subsystem_locator_->Shutdown();
	}

	void Engine::Run()
	{
		auto platform = subsystem_locator_->GetSubsystem<platform::Platform>();
		auto graphics = subsystem_locator_->GetSubsystem<graphics::GraphicsLibrary>();
		auto timer = subsystem_locator_->GetSubsystem<Timer>();
		auto input_manager = subsystem_locator_->GetSubsystem<input::InputManager>();

		//Main Loop
		while (!platform->GetRequestShutdown())
		{
			if(!platform->ProcessSystemEventQueue())
			{
				input_manager->Update();
				timer->Tick();

				timer->ShowFrameTime(platform->GetHwnd());


				//描画
				graphics->FrameBegin();
				graphics->FrameEnd();
			}
		}
	}


}
