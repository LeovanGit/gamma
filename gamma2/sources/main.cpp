#include "types.h"
#include "window.h"
#include "screen_capturer.h"

struct HotKey
{
	HotKey(
		int key_,
		std::function<void()> onPressAction_,
		std::function<void()> onReleaseAction_)
		: key(key_)
		, wasPressed(false)
		, onPressAction(onPressAction_)
		, onReleaseAction(onReleaseAction_)
	{

	}

	int key;
	// To avoid repeated activations while finger still on the button:
	bool wasPressed;
	std::function<void()> onPressAction;
	std::function<void()> onReleaseAction;
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

	bool completelyDisabled = true;
	int lastPressedUIKey = -1;
	
	bool wantWindowVisible = false;

	auto UIButtonAction =
		[&window,
		&lastPressedUIKey,
		&completelyDisabled,
		&wantWindowVisible](int key)
	{
		if (completelyDisabled) return;

		if (lastPressedUIKey == key ||
			(key == VK_ESCAPE && lastPressedUIKey != VK_ESCAPE))
		{
			// Wait until in-game UI window will close
			// (to avoid flashbang effect):
			Sleep(300);

			wantWindowVisible = true;
			lastPressedUIKey = -1;
		}
		else
		{
			wantWindowVisible = false;
			lastPressedUIKey = key;
		}
	};

	std::vector<HotKey> keys;
	keys.emplace_back(
		'Z',
		[&window,
		&completelyDisabled,
		&lastPressedUIKey,
		&wantWindowVisible]()
		{
			wantWindowVisible = completelyDisabled;
			completelyDisabled = !completelyDisabled;
			lastPressedUIKey = -1;
		},
		nullptr);

	// Crafts:
	keys.emplace_back(
		'Q',
		[&UIButtonAction]() { UIButtonAction('Q'); },
		nullptr);

	// Inventory:
	keys.emplace_back(
		VK_TAB,
		[&UIButtonAction]() { UIButtonAction(VK_TAB); },
		nullptr);

	// Menu:
	keys.emplace_back(
		VK_ESCAPE,
		[&UIButtonAction]() { UIButtonAction(VK_ESCAPE); },
		nullptr);

	// Map (requires special handling, because map is open until we hold 'G' key
	// (unlike others keys), so we need to return window visibility back on key release):
	keys.emplace_back(
		'G',
		[&UIButtonAction]() { UIButtonAction('G'); },
		[&UIButtonAction]() { UIButtonAction('G'); });

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

			if (isCurrentlyPressed && !key.wasPressed && key.onPressAction)
			{
				key.onPressAction();
			}

			if (!isCurrentlyPressed && key.wasPressed && key.onReleaseAction)
			{
				key.onReleaseAction();
			}

			key.wasPressed = isCurrentlyPressed;
		}

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
			Sleep(10); // to avoid 100% CPU load
		}

		window.SetVisibility(wantWindowVisible);
    }

    return 0;
}
