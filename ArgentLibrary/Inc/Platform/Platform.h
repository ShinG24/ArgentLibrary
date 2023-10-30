#pragma once

#include <Windows.h>

namespace argent::platform
{
	class Platform
	{
	public:
		Platform() = default;
		~Platform() = default;

		void Awake(long window_width, long window_height);

		bool ProcessSystemEventQueue();

		HWND GetHwnd() const { return hwnd_; }
		bool GetRequestShutdown() const { return request_shutdown_; }
	private:
		HWND hwnd_;
		bool request_shutdown_;
	};
}