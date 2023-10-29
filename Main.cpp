
#include <crtdbg.h>

#include "ArgentGraphicsLibrary/Inc/ArgentGraphicsLibrary.h"
#include "ArgentGraphicsLibrary/Inc/Platform.h"
#include "ArgentUtilityLibrary/Inc/Timer.h"
#include "ArgentInputLibrary/Inc/InputManager.h"

#include "Scene.h"


#ifdef _DEBUG
int main()
#else
int WINAPI wWinMain(
	_In_ HINSTANCE h_instance,
	_In_opt_ HINSTANCE h_prevInstance,
	_In_ LPWSTR lp_cmdLine,
	_In_ int n_show_cmd
	)
#endif
{
#ifdef _DEBUG 
	//Detect memory leak.
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(0x00);
#endif

	//ウィンドウの作成
	argent::platform::Platform platform{};
	platform.Awake(1280, 720);

	//描画ライブラリの起動
	argent::graphics::GraphicsLibrary graphics_library{};
	graphics_library.Awake(platform.GetHwnd());

	//タイマー
	argent::Timer timer;
	timer.Awake();

	//インプット
	argent::input::InputManager input_manager;
	input_manager.Awake(platform.GetHwnd());

	Scene demo_scene;
	demo_scene.Awake();

	//Main Loop
	while (!platform.GetRequestShutdown())
	{
		if(!platform.ProcessSystemEventQueue())
		{
			input_manager.Update();
			timer.Tick();

			timer.ShowFrameTime(platform.GetHwnd());

			demo_scene.Update();

			//描画
			graphics_library.FrameBegin();
			demo_scene.Render();
			graphics_library.FrameEnd();
		}
	}

	demo_scene.Shutdown();
	graphics_library.Shutdown();

	return 0;
}