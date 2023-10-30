#pragma once

#include "Keyboard.h"
#include "Mouse.h"
#include "GamePad.h"

namespace argent::input
{
	class InputManager
	{
	public:
		InputManager() = default;
		~InputManager() = default;

		InputManager(const InputManager&) = delete;
		InputManager(const InputManager&&) = delete;
		InputManager& operator=(const InputManager&) = delete;
		InputManager& operator=(const InputManager&&) = delete;

		static InputManager* Get(){ return instance_; }

		void Awake(HWND hwnd);
		void Shutdown();
		void Update();

		const Keyboard* GetKeyboard() const { return &keyboard_; }
		const Mouse* GetMouse() const { return &mouse_; }
	private:
		static InputManager* instance_;
		HWND hwnd_;
		Keyboard keyboard_;
		Mouse mouse_;
		GamePad game_pad_;
	};
}