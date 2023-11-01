
#include <crtdbg.h>

#include "ArgentLibrary.h"

#include "ArgentLibrary/Inc/Subsystem/Graphics/GraphicsLibrary.h"
#include "ArgentLibrary/Inc/Subsystem/Platform/Platform.h"
#include "ArgentLibrary/Inc/Subsystem/Timer/Timer.h"
#include "ArgentLibrary/Inc/Subsystem/Input/InputManager.h"

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

	argent::Initialize(1280, 720);

	argent::scene_management::RegisterScene("Demo", new DemoScene);
	argent::scene_management::SetNextScene("Demo");
	argent::AppRun();

	argent::Finalize();

	return 0;
}