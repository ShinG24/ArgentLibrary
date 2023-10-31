#include "ArgentLibrary.h"

#include <memory>

#include "Core/SubsystemLocator.h"
#include "Core/Engine.h"

#include "Subsystem/Platform/Platform.h"
#include "Subsystem/Graphics/GraphicsLibrary.h"
#include "Subsystem/Input/InputManager.h"
#include "Subsystem/Timer/Timer.h"

#include "Subsystem/Scene/BaseScene.h"
#include "Subsystem/Scene/SceneManager.h"

namespace argent
{
	/**
	 * \brief ライブラリに必要な機能をまとめた場所
	 */
	class ArgentLibrary
	{
	public:
		ArgentLibrary();
		~ArgentLibrary() = default;

		ArgentLibrary(ArgentLibrary&) = delete;
		ArgentLibrary(ArgentLibrary&&) = delete;
		ArgentLibrary& operator=(ArgentLibrary&) = delete;
		ArgentLibrary& operator=(ArgentLibrary&&) = delete;

		void Awake(long window_width, long window_height) const;
		void Shutdown() const;

		void Run() const;

		Engine* GetEngine() const { return engine_.get(); }
	private:

		std::unique_ptr<Engine> engine_{};

	};

	ArgentLibrary::ArgentLibrary() :
		engine_(std::make_unique<Engine>())
	{}

	void ArgentLibrary::Awake(long window_width, long window_height) const
	{
		engine_->Initialize(window_width, window_height);
	}

	void ArgentLibrary::Shutdown() const
	{
		engine_->Finalize();
	}

	void ArgentLibrary::Run() const
	{
		engine_->Run();
	}

	static ArgentLibrary library_instance;

	//-------------------------------------------Wrap Function-------------------------------------------//

	void Initialize(long window_width, long window_height)
	{
		library_instance.Awake(window_width, window_height);
	}

	void Finalize()
	{
		library_instance.Shutdown();
	}

	void AppRun()
	{
		library_instance.Run();
	}


	
}

namespace argent::scene_management
{
	void RegisterScene(std::string scene_name, scene::BaseScene* scene)
	{
		library_instance.GetEngine()->GetSubsystemLocator()->GetSubsystem<scene::SceneManager>()->Register(scene_name, scene);
	}

	void SetNextScene(std::string scene_name)
	{
		library_instance.GetEngine()->GetSubsystemLocator()->GetSubsystem<scene::SceneManager>()->SetNextScene(scene_name);
	}
}

