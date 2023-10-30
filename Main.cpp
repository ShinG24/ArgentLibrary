
#include <crtdbg.h>

#include "ArgentLibrary/Inc/Graphics/ArgentGraphicsLibrary.h"
#include "ArgentLibrary/Inc/Platform/Platform.h"
#include "ArgentLibrary/Inc/Timer/Timer.h"
#include "ArgentLibrary/Inc/Input/InputManager.h"

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

	//�E�B���h�E�̍쐬
	argent::platform::Platform platform{};
	platform.Awake(1280, 720);

	//�`�惉�C�u�����̋N��
	std::unique_ptr<argent::graphics::GraphicsLibrary> graphics_library = std::make_unique<argent::graphics::GraphicsLibrary>(platform.GetHwnd());
	graphics_library->Awake(platform.GetHwnd());

	//�^�C�}�[
	argent::Timer timer;
	timer.Awake();

	//�C���v�b�g
	argent::input::InputManager input_manager;
	input_manager.Awake(platform.GetHwnd());

	std::unique_ptr<Scene> demo_scene = std::make_unique<Scene>();
	demo_scene->Awake();

	//Main Loop
	while (!platform.GetRequestShutdown())
	{
		if(!platform.ProcessSystemEventQueue())
		{
			input_manager.Update();
			timer.Tick();

			timer.ShowFrameTime(platform.GetHwnd());

			demo_scene->Update();

			//�`��
			graphics_library->FrameBegin();
			demo_scene->Render();
			graphics_library->FrameEnd();
		}
	}

	demo_scene->Shutdown();

	graphics_library->Shutdown();

	return 0;
}