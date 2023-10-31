#include "Subsystem/Input/InputManager.h"

#include "Core/SubsystemLocator.h"
#include "Core/Engine.h"

#include "Subsystem/Platform/Platform.h"

namespace argent::input
{
	void InputManager::Awake()
	{
		hwnd_ = hwnd_ = GetEngine()->GetSubsystemLocator()->GetSubsystem<platform::Platform>()->GetHwnd();
		keyboard_.Awake();
		mouse_.Awake();
		Subsystem::Awake();
	}

	void InputManager::Shutdown()
	{
		
	}

	void InputManager::Update()
	{
		keyboard_.Update();
		mouse_.Update(hwnd_);
	}
}