#pragma once

#include "types.h"
#include "hotkeys.h"

int GetMillisecondsDelayForButton(int key);

class HotkeysController
{
public:
	HotkeysController();

	void InitHotkeys();

	void ProcessHotkeys();

	bool WantWindowVisible() const;

private:
	void OnMainButton();
	void OnGameUIButton(int key);
	void OnEscapeButton();

	Hotkeys hotkeys;

	bool m_completelyDisabled; // don't react on other keys if window was hidden by mainKey key
	bool m_wantWindowVisible;
	int m_lastPressedUIKey;
};
