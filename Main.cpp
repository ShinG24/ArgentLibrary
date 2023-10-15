
#include <crtdbg.h>

#include "ArgentGraphicsLibrary/Inc/ArgentGraphicsLibrary.h"
#include "ArgentGraphicsLibrary/Inc/Platform.h"
#include "ArgentUtilityLibrary/Inc/Timer.h"


int main()
{
#ifdef _DEBUG 
	//Detect memory leak.
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(0x00);
#endif

	argent::platform::Platform platform{};
	platform.Awake(1280, 720);

	argent::graphics::GraphicsLibrary graphics_library{};
	graphics_library.Awake(platform.GetHwnd());
	argent::Timer timer;
	timer.Awake();

	while (!platform.GetRequestShutdown())
	{
		if(!platform.ProcessSystemEventQueue())
		{
			timer.Tick();
#ifdef _DEBUG
			timer.ShowFrameTime(platform.GetHwnd());
#endif
			graphics_library.FrameBegin();
			graphics_library.FrameEnd();
		}
	}

	graphics_library.Shutdown();

	return 0;
}