
#include <crtdbg.h>

#include "ArgentGraphicsLibrary/Inc/ArgentGraphicsLibrary.h"
#include "ArgentPlatformLibrary/Inc/Platform.h"


int main()
{
#ifdef _DEBUG 
	//ÉÅÉÇÉäÉäÅ[ÉNÇÃåüèo
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(0x00);
#endif

	argent::platform::Platform platform{};
	platform.Awake(1280, 720);

	argent::graphics::GraphicsLibrary graphics_library{};
	graphics_library.Awake(platform.GetHwnd());

	while (!platform.GetRequestShutdown())
	{
		platform.ProcessSystemEventQueue();
		graphics_library.FrameBegin();

		graphics_library.FrameEnd();
	}
	return 0;
}