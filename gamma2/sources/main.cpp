#include "types.h"
#include "window.h"
#include "screen_capturer.h"

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
    Window window(0, 200, 1920, 680, WindowProc, hInstance);
    
    ScreenRect rect = { 0, 200, 1920, 680 };
    ScreenCapturer sc(rect);

    MSG msg;
    while (true)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) return msg.wParam;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        sc.TakeScreenshot();
        //sc.ApplyGamma();

        window.ShowImage(sc.GetScreenshot());

        Sleep(10); // to avoid 100% CPU load
    }

    return 0;
}
