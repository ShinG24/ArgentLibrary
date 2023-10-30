#include "Timer/Timer.h"

#include <sstream>

namespace argent
{
	Timer* Timer::instance_ = nullptr;
	Timer::Timer()
	{
		instance_ = this;
	}

	void Timer::Awake()
	{
		LONGLONG counts_per_second{};

		//how many can count in one second.
		QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&counts_per_second));

		//how much time does it need to  one count.
		seconds_per_count_ = 1.0 / static_cast<double>(counts_per_second);

		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&this_time_));

		base_time_ = this_time_;
		last_time_ = this_time_;
		delta_time_ = 0;
		elapsed_time_ = 0;
	}

	void Timer::Tick()
	{
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&this_time_));

		delta_time_ = static_cast<double>(this_time_ - last_time_) * seconds_per_count_;

		last_time_ = this_time_;

		if(delta_time_ < 0.0)
		{
			delta_time_ = 0;
		}
	}

	void Timer::ShowFrameTime(HWND hwnd)
	{
		const float stamp = static_cast<float>(static_cast<double>(this_time_ - base_time_) * seconds_per_count_);
		++frames_;

		if(stamp - elapsed_time_ >= 1.0f)
		{
			const float fps = static_cast<float>(frames_);
			const float frame_times = 1000.0f / fps;
			frame_per_seconds_  = static_cast<int>(fps);

#ifdef _DEBUG
			std::wostringstream outs;
			outs.precision(6);
			outs << L"FPS : " << frame_per_seconds_ << L"/" << L"Frame Time : " << frame_times << L"(ms)";
			SetWindowTextW(hwnd, outs.str().c_str());
#endif
			frames_ = 0;
			elapsed_time_ += 1.0f;
		}
	}
}
