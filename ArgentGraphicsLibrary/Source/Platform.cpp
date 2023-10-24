#include "../Inc/Platform.h"

#include <crtdbg.h>
#include <Windows.h>

	extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
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
		ShowWindow(hwnd_, SW_MAXIMIZE);
	}

	bool Platform::ProcessSystemEventQueue()
	{
		MSG msg{};
		bool peek_message = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
		if(peek_message)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if(WM_QUIT == msg.message) request_shutdown_ = true;
		return peek_message;
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

		ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}