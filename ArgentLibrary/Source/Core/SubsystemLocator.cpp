#include "Core/SubsystemLocator.h"

#include "Subsystem/Platform/Platform.h"
#include "Subsystem/Graphics/GraphicsLibrary.h"
#include "Subsystem/Input/InputManager.h"
#include "Subsystem/Timer/Timer.h"


namespace argent
{
	void SubsystemLocator::Awake()
	{
		AddSubsystem<platform::Platform>();
		AddSubsystem<graphics::GraphicsLibrary>();
		AddSubsystem<input::InputManager>();
		AddSubsystem<Timer>();
	}

	void SubsystemLocator::Shutdown()
	{
		GetSubsystem<Timer>()->Shutdown();
		GetSubsystem<input::InputManager>()->Shutdown();
		GetSubsystem<graphics::GraphicsLibrary>()->Shutdown();
		GetSubsystem<platform::Platform>()->Shutdown();
	}
}
