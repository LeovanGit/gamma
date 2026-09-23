#include "types.h"
#include "window.h"
#include "screen_capturer.h"

struct HotKey
{
	HotKey(
		int key_,
		std::function<void()> action_)
		: key(key_),
		wasPressed(false),
		action(action_)
	{

	}

	int key;
	// To avoid repeated activations while finger still on the button:
	bool wasPressed;
	std::function<void()> action;
};

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

	std::vector<HotKey> keys;
	keys.emplace_back(
		'Z',
		[&window]()
		{
			window.ToggleVisibility();
		});

    MSG msg;
    while (true)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) return msg.wParam;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		for (auto& key : keys)
		{
			bool isCurrentlyPressed = (GetAsyncKeyState(key.key) & 0x8000);

			if (isCurrentlyPressed && !key.wasPressed)
			{
				key.action();
			}

			key.wasPressed = isCurrentlyPressed;
		}

        sc.Render();
    }

    return 0;
}
