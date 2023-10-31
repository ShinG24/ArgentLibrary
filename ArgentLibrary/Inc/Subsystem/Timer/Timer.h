#pragma once

#include <Windows.h>

#include "Subsystem/Subsystem.h"

namespace argent
{
	class Timer final : public Subsystem
	{
	public:
		Timer();
		~Timer() override = default;

		static Timer* Get() { return instance_; }
		void Awake() override;
		void Tick();

		void ShowFrameTime(HWND hwnd_);

		float GetDeltaTime() const { return static_cast<float>(delta_time_); }
		int GetFps() const { return frame_per_seconds_; }

	private:

		//TODO è¡ÇµÇ»Ç≥Ç¢
		static Timer* instance_;
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