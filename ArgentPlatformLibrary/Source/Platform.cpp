#include "../Inc/Platform.h"

#include <crtdbg.h>
#include <Windows.h>

namespace argent::platform
{
	LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void Platform::Awake(long window_width, long window_height)
	{
		request_shutdown_ = false;

		//Creating and registering window
		WNDCLASSEXW wc{};
		wc.cbSize = sizeof(wc);
		wc.lpfnWndProc = HandleMessage;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetModuleHandle(nullptr);
		wc.lpszClassName = L"Window";


		if (!RegisterClassExW(&wc)) _ASSERT_EXPR(FALSE, L"Already registered");

		RECT rc{ 0, 0, window_width, window_height };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		DWORD window_style = WS_OVERLAPPEDWINDOW/* ^ WS_MAXIMIZEBOX*/ ^ WS_THICKFRAME;

		hwnd_ = CreateWindowExW(0, wc.lpszClassName, L"Argent Engine",
			window_style, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,
			rc.bottom - rc.top, nullptr, nullptr,
			wc.hInstance, nullptr);

		ShowWindow(hwnd_, SW_SHOW);
	}

	void Platform::ProcessSystemEventQueue()
	{
		MSG msg{};

		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if(WM_QUIT == msg.message) request_shutdown_ = true;
	}

	LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_DESTROY:
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;

		default:
			break;
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}