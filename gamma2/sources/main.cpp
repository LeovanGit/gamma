#include "screen_capturer.h"
#include "window.h"
#include "hotkeys.h"
#include "types.h"

namespace
{
    constexpr int mainKey = 'Z';
} // namespace

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);

        break;
    }
    }

    // Handle any messages the switch statement didn't:
    return DefWindowProc(hwnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    uint16_t screenWidth = GetSystemMetrics(SM_CXSCREEN);
    uint16_t screenHeight = GetSystemMetrics(SM_CYSCREEN);

    Window window(0, 0, screenWidth, screenHeight, WindowProc, hInstance);
    ScreenCapturer sc(window);

    bool wantWindowVisible = false;

    Hotkeys hotkeys;

    hotkeys.AddKey(
        mainKey,
        [&]() { wantWindowVisible = !wantWindowVisible; },
        nullptr);

    MSG msg;
    while (true)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) return msg.wParam;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		hotkeys.ProcessKeys();

		// We need to render new frame BEFORE we unhide window
		// (because when window is hidden, we don't render and
		// swapchain contains garbage - old/prev. frame, which
		// can cause flashbang effect), so we will just set
		// wantWindowVisible bool in keys loop and actually
		// show window only after render:
		if (wantWindowVisible)
		{
			sc.Render();
		}
		else
		{
			// To avoid 100% CPU load - wait any window MSG with 10 ms timeout:
            MsgWaitForMultipleObjectsEx(0, nullptr, 10, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
		}

		window.SetVisibility(wantWindowVisible);
    }

    return 0;
}
