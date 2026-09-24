#pragma once

#include "types.h"

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

class Hotkeys
{
public:
	Hotkeys() = default;

	void AddKey(
		int key,
		std::function<void()> onPressAction,
		std::function<void()> onReleaseAction);

	void ProcessKeys();

private:
	std::vector<HotKey> keys;
};
