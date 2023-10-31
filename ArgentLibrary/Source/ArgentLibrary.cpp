//#include "ArgentLibrary.h"
//
//#include <memory>
//
//#include "Platform/Platform.h"
//#include "Graphics/GraphicsLibrary.h"
//#include "Input/InputManager.h"
//#include "Timer/Timer.h"
//
//namespace argent
//{
//	/**
//	 * \brief ライブラリに必要な機能をまとめた場所
//	 */
//	class ArgentLibrary
//	{
//	public:
//		ArgentLibrary() = default;
//		~ArgentLibrary() = default;
//
//		ArgentLibrary(ArgentLibrary&) = delete;
//		ArgentLibrary(ArgentLibrary&&) = delete;
//		ArgentLibrary& operator=(ArgentLibrary&) = delete;
//		ArgentLibrary& operator=(ArgentLibrary&&) = delete;
//
//		void Awake();
//		void Shutdown();
//
//		void Run();
//
//	private:
//		std::unique_ptr<platform::Platform> platform_;
//		std::unique_ptr<graphics::GraphicsLibrary> graphics_library_;
//		std::unique_ptr<input::InputManager> input_manager_;
//		std::unique_ptr<Timer> hi_resolution_timer_;
//	};
//
//	void ArgentLibrary::Awake()
//	{
//		platform_ = std::make_unique<platform::Platform>();
//		graphics_library_ = std::make_unique<graphics::GraphicsLibrary>(platform_->GetHwnd());
//		input_manager_ = std::make_unique<input::InputManager>();
//		hi_resolution_timer_ = std::make_unique<Timer>();
//
//		platform_->Awake()
//	}
//
//	static ArgentLibrary library_instance;
//
//	//-------------------------------------------Wrap Function-------------------------------------------//
//
//	void Initialize()
//	{
//		library_instance.Awake();
//	}
//
//	void Finalize()
//	{
//		library_instance.Shutdown();
//	}
//
//	void AppRun()
//	{
//		library_instance.Run();
//	}
//}
