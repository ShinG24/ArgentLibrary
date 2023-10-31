#pragma once

#include "Subsystem/Subsystem.h"

#include "Keyboard.h"
#include "Mouse.h"
#include "GamePad.h"

namespace argent::input
{
	class InputManager final : public Subsystem
	{
	public:

		InputManager() = default;
		~InputManager() override = default;

		InputManager(const InputManager&) = delete;
		InputManager(const InputManager&&) = delete;
		InputManager& operator=(const InputManager&) = delete;
		InputManager& operator=(const InputManager&&) = delete;

		void Awake() override;
		void Shutdown() override;
		void Update();

		const Keyboard* GetKeyboard() const { return &keyboard_; }
		const Mouse* GetMouse() const { return &mouse_; }

	private:

		HWND hwnd_;
		Keyboard keyboard_;
		Mouse mouse_;
		GamePad game_pad_;
	};
}