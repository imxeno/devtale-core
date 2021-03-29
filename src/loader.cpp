#include <thread>
#include <Windows.h>
#include "resource.h"

#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)
#define UNUSED(x) (void*)(x)

static const char* window_class_name = "devtale-core";
HWND splash_window;
bool splash_visible = false;
HINSTANCE h_instance;

LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void splash()
{
	splash_visible = true;
	WNDCLASSEX window_class;
	HBITMAP h_splash = LoadBitmap(h_instance, MAKEINTRESOURCE(IDB_SPLASH));
	BITMAP splash;
	GetObject(h_splash, sizeof(BITMAP), &splash);

	window_class.cbSize = sizeof(WNDCLASSEX);
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = wnd_proc;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = h_instance;
	window_class.hIcon = LoadIcon(h_instance, MAKEINTRESOURCE(IDI_APPLICATION));
	window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
	window_class.hbrBackground = CreatePatternBrush(h_splash);
	window_class.lpszMenuName = nullptr;
	window_class.lpszClassName = window_class_name;
	window_class.hIconSm = LoadIcon(window_class.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));


	if (!RegisterClassEx(&window_class))
	{
		return;
	}

	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);

	splash_window = CreateWindowA(
		window_class_name,
		"DevTale Core",
		WS_POPUP,
		(desktop.right - splash.bmWidth) / 2, (desktop.bottom - splash.bmHeight) / 2,
		splash.bmWidth, splash.bmHeight,
		NULL,
		NULL,
		h_instance,
		NULL
	);

	if (!splash_window)
	{
		return;
	}

	ShowWindow(splash_window, SW_SHOW);
	UpdateWindow(splash_window);

	while (splash_visible)
		Sleep(10);

	DestroyWindow(splash_window);
	UnregisterClass(window_class_name, h_instance);
}

std::thread splash_thread;

// ReSharper disable once CppInconsistentNaming
EXTERN_DLL_EXPORT void ShowNostaleSplash()
{
	if (splash_visible) return;
	splash_thread = std::thread(&splash);
}

// ReSharper disable once CppInconsistentNaming
EXTERN_DLL_EXPORT void FreeNostaleSplash()
{
	splash_visible = false;
	splash_thread.join();
}

// ReSharper disable once CppInconsistentNaming
bool WINAPI DllMain(_In_ HINSTANCE instance, _In_ DWORD call_reason, _In_ LPVOID reserved)
{
	UNUSED(instance);
	UNUSED(reserved);

	switch (call_reason)
	{
	case DLL_PROCESS_ATTACH:
		h_instance = instance;
		LoadLibraryA("devtale-core.dll");
		break;
	default:
		break;
	}
	return TRUE;
}
