#include "hotkeys.h"

void Hotkeys::AddKey(
	int key,
	std::function<void()> onPressAction,
	std::function<void()> onReleaseAction)
{
	keys.emplace_back(key, onPressAction, onReleaseAction);
}

void Hotkeys::ProcessKeys()
{
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
}
