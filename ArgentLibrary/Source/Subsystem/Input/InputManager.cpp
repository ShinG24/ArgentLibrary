#include "Subsystem/Input/InputManager.h"


namespace argent::input
{
	InputManager* InputManager::instance_ = nullptr;
	void InputManager::Awake(HWND hwnd)
	{
		instance_ = this;
		hwnd_ = hwnd;
		keyboard_.Awake();
		mouse_.Awake();
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