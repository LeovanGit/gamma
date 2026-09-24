#include "hotkeys_controller.h"

namespace
{
	constexpr int VK_NULL = -1;

	constexpr int mainKey = 'Z';

	constexpr int craftsKey = 'Q';
	constexpr int mapKey = 'G';
	constexpr int inventoryKey = VK_TAB;
	constexpr int escMenuKey = VK_ESCAPE;
} // namespace

int GetMillisecondsDelayForButton(int key)
{
	switch (key)
	{
	case (mainKey):
	{
		return 0;
	}
	case (escMenuKey):
	{
		return 100;
	}
	case (inventoryKey):
	case (craftsKey):
	{
		return 200;
	}
	case (mapKey):
	{
		return 300;
	}
	default:
	{
		assert(false && "Unknown key");
		return 0;
	}
	}
}

HotkeysController::HotkeysController()
	: m_completelyDisabled(true)
	, m_wantWindowVisible(false)
	, m_lastPressedUIKey(VK_NULL)
{
	InitHotkeys();
}

void HotkeysController::InitHotkeys()
{
	hotkeys.AddKey(
		mainKey,
		[this]() { OnMainButton(); },
		nullptr);

	hotkeys.AddKey(
		craftsKey,
		[this]() { OnGameUIButton(craftsKey); },
		nullptr);

	hotkeys.AddKey(
		inventoryKey,
		[this]() { OnGameUIButton(inventoryKey); },
		nullptr);

	hotkeys.AddKey(
		escMenuKey,
		[this]() { OnEscapeButton(); },
		nullptr);

	// mapKey requires special handling, because map is open until we hold mapKey key
	// (unlike others keys), so we need to return window visibility back on key release):
	hotkeys.AddKey(
		mapKey,
		[this]() { OnGameUIButton(mapKey); },
		[this]() { OnGameUIButton(mapKey); });
}

void HotkeysController::ProcessHotkeys()
{
	hotkeys.ProcessKeys();
}

bool HotkeysController::WantWindowVisible() const
{
	return m_wantWindowVisible;
}

// mainKey key:
void HotkeysController::OnMainButton()
{
	m_wantWindowVisible = m_completelyDisabled;
	m_completelyDisabled = !m_completelyDisabled;
	m_lastPressedUIKey = VK_NULL;
}

// To hide our window when we open Inventory/Crafts/Map in-game UI:
void HotkeysController::OnGameUIButton(int key)
{
	if (m_completelyDisabled) return;

	if (m_lastPressedUIKey != key)
	{
		m_wantWindowVisible = false;
		m_lastPressedUIKey = key;
	}
	else
	{
		// Wait until in-game UI window will close
		// (to avoid flashbang effect):
		int delay = GetMillisecondsDelayForButton(key);
		if (delay > 0) Sleep(delay);

		m_wantWindowVisible = true;
		m_lastPressedUIKey = VK_NULL;
	}
}

// We also can close some in-game UIs using ESC instead of their keys,
// so we need to handle it separately:
void HotkeysController::OnEscapeButton()
{
	if (m_completelyDisabled) return;

	// Close other UI windows if they are open:
	if (m_lastPressedUIKey == escMenuKey ||
		m_lastPressedUIKey == craftsKey ||
		m_lastPressedUIKey == inventoryKey)
	{
		// Wait until in-game UI window will close
		// (to avoid flashbang effect):
		int delay = GetMillisecondsDelayForButton(m_lastPressedUIKey);
		if (delay > 0) Sleep(delay);

		m_wantWindowVisible = true;
		m_lastPressedUIKey = VK_NULL;
	}
	// Open ESC menu:
	else
	{
		m_wantWindowVisible = false;
		m_lastPressedUIKey = escMenuKey;
	}
}
