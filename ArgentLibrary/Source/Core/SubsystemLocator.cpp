#include "Core/SubsystemLocator.h"

#include "Subsystem/Platform/Platform.h"
#include "Subsystem/Graphics/GraphicsLibrary.h"
#include "Subsystem/Input/InputManager.h"
#include "Subsystem/Timer/Timer.h"

#include "Subsystem/Graphics/Wrapper/DXR/ShaderLibrary.h"
#include "Subsystem/Graphics/Wrapper/DXR/ShaderLibraryManager.h"

#include "Subsystem/Scene/BaseScene.h"
#include "Subsystem/Scene/SceneManager.h"


namespace argent
{
	void SubsystemLocator::Awake()
	{
		AddSubsystem<platform::Platform>();
		AddSubsystem<graphics::GraphicsLibrary>();
		AddSubsystem<input::InputManager>();
		AddSubsystem<Timer>();
		AddSubsystem<scene::SceneManager>();
	}

	void SubsystemLocator::Shutdown()
	{
		GetSubsystem<scene::SceneManager>()->Shutdown();
		GetSubsystem<Timer>()->Shutdown();
		GetSubsystem<input::InputManager>()->Shutdown();
		GetSubsystem<graphics::GraphicsLibrary>()->Shutdown();
		GetSubsystem<platform::Platform>()->Shutdown();
	}
}
