#pragma once

#include <Windows.h>

#include "Subsystem/Subsystem.h"

namespace argent::platform
{
	class Platform final : public Subsystem
	{
	public:
		Platform() = default;
		~Platform() override = default;

		void Awake() override;

		bool ProcessSystemEventQueue();

		HWND GetHwnd() const { return hwnd_; }
		bool GetRequestShutdown() const { return request_shutdown_; }
	private:
		HWND hwnd_;
		bool request_shutdown_;
	};
}