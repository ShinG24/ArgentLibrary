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

		void Awake(long window_width, long window_height);
		void Shutdown() const;

		void Run() const;

		Engine* GetEngine() const { return engine_.get(); }

		input::InputManager* GetInputManager() const { return input_manager_.get(); }
	private:

		std::unique_ptr<Engine> engine_{};
		std::shared_ptr<input::InputManager> input_manager_;
	};

	ArgentLibrary::ArgentLibrary() :
		engine_(std::make_unique<Engine>())
	{}

	void ArgentLibrary::Awake(long window_width, long window_height)
	{
		engine_->Initialize(window_width, window_height);
		input_manager_ = engine_->GetSubsystemLocator()->GetSubsystem<input::InputManager>();
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

namespace argent::input
{
	bool input::GetKey(KeyCode key)
	{
		return library_instance.GetInputManager()->GetKeyboard()->GetKey(key);
	}

	bool GetButton(MouseButton button)
	{
		return library_instance.GetInputManager()->GetMouse()->GetButton(button);
	}

	float GetMouseMoveX()
	{
		return library_instance.GetInputManager()->GetMouse()->GetMovedVec().x;
	}
	float GetMouseMoveY()
	{
		return library_instance.GetInputManager()->GetMouse()->GetMovedVec().y;
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

