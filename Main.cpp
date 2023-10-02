
#include <crtdbg.h>
#include <iostream>
#include <Windows.h>

#include "ArgentGraphicsLibrary/Inc/ArgentGraphicsLibrary.h"

LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


HWND CreateAppWindow(long window_width, long window_height);


int main()
{
	HWND hwnd = CreateAppWindow(1280, 720);

	argent::graphics::GraphicsLibrary graphics_library;
	graphics_library.Awake(hwnd);

	while (true)
	{
		std::string s;
		std::cin >> s;

		

		break;
	}
	return 0;
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

HWND CreateAppWindow(long window_width, long window_height)
{
	HWND hwnd{};
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

	hwnd = CreateWindowExW(0, wc.lpszClassName, L"Argent Engine",
		window_style, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,
		rc.bottom - rc.top, nullptr, nullptr,
		wc.hInstance, nullptr);

	ShowWindow(hwnd, SW_SHOW);
	return hwnd;
}
