#include "ArgentLibrary.h"

#include <memory>

#include "Core/SubsystemLocator.h"
#include "Core/Engine.h"

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

		std::unique_ptr<Engine> engine_;
	};

	void ArgentLibrary::Awake(long window_width, long window_height)
	{
		engine_ = std::make_unique<Engine>();
		engine_->Initialize(window_width, window_height);
	}

	void ArgentLibrary::Shutdown()
	{
		engine_->Finalize();
	}

	void ArgentLibrary::Run()
	{
		engine_->Run();
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
