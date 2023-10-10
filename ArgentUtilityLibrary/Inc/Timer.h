#pragma once

#include <Windows.h>
namespace argent
{
	class Timer
	{
	public:
		Timer();
		~Timer() = default;

		void Awake();
		void Tick();

		void ShowFrameTime(HWND hwnd_);

		float GetDeltaTime() const { return static_cast<float>(delta_time_); }
	private:

		double delta_time_;
		double seconds_per_count_;
		float elapsed_time_;
		UINT frames_;

		LONGLONG this_time_;
		LONGLONG last_time_;
		LONGLONG base_time_;
		int frame_per_seconds_;
	};
}