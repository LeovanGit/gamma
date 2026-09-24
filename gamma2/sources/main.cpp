#include "screen_capturer.h"
#include "window.h"
#include "hotkeys_controller.h"
#include "types.h"

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
    Window window(0, 0, 1920, 1080, WindowProc, hInstance);
    ScreenCapturer sc(window);

    HotkeysController hotkeys;

    MSG msg;
    while (true)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) return msg.wParam;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		hotkeys.ProcessHotkeys();

		// We need to render new frame BEFORE we unhide window
		// (because when window is hidden, we don't render and
		// swapchain contains garbage - old/prev. frame, which
		// can cause flashbang effect), so we will just set
		// wantWindowVisible bool in keys loop and actually
		// show window only after render:
        bool wantWindowVisible = hotkeys.WantWindowVisible();

		if (wantWindowVisible)
		{
			sc.Render();
		}
		else
		{
			Sleep(10); // to avoid 100% CPU load
		}

		window.SetVisibility(wantWindowVisible);
    }

    return 0;
}
