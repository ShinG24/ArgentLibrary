#include "ArgentLibrary.h"

#include <memory>

#include "Subsystem/Platform/Platform.h"
#include "Subsystem/Graphics/GraphicsLibrary.h"
#include "Subsystem/Input/InputManager.h"
#include "Subsystem/Timer/Timer.h"

namespace argent
{
	/**
	 * \brief ライブラリに必要な機能をまとめた場所
	 */
	class ArgentLibrary
	{
	public:
		ArgentLibrary() = default;
		~ArgentLibrary() = default;

		ArgentLibrary(ArgentLibrary&) = delete;
		ArgentLibrary(ArgentLibrary&&) = delete;
		ArgentLibrary& operator=(ArgentLibrary&) = delete;
		ArgentLibrary& operator=(ArgentLibrary&&) = delete;

		void Awake(long window_width, long window_height);
		void Shutdown();

		void Run();

	private:
		std::unique_ptr<platform::Platform> platform_;
		std::unique_ptr<graphics::GraphicsLibrary> graphics_library_;
		std::unique_ptr<input::InputManager> input_manager_;
		std::unique_ptr<Timer> hi_resolution_timer_;
	};

	void ArgentLibrary::Awake(long window_width, long window_height)
	{
		platform_ = std::make_unique<platform::Platform>();
		graphics_library_ = std::make_unique<graphics::GraphicsLibrary>();
		input_manager_ = std::make_unique<input::InputManager>();
		hi_resolution_timer_ = std::make_unique<Timer>();

		platform_->Awake(window_width, window_height);
		graphics_library_->Awake(platform_->GetHwnd());
		input_manager_->Awake(platform_->GetHwnd());
		hi_resolution_timer_->Awake();
	}

	void ArgentLibrary::Shutdown()
	{
		input_manager_->Shutdown();
		graphics_library_->Shutdown();
	}

	void ArgentLibrary::Run()
	{
		while (!platform_->GetRequestShutdown())
		{
			
		}
	}

	static ArgentLibrary library_instance;

	//-------------------------------------------Wrap Function-------------------------------------------//

	void Initialize(long window_width, long window_height)
	{
		library_instance.Awake(window_width, window_height);
	}

	void Finalize()
	{
		library_instance.Shutdown();
	}

	void AppRun()
	{
		library_instance.Run();
	}
}
