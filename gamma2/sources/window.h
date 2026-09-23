#pragma once

#include "types.h"

class Window
{
public:
	Window(
		uint16_t x,
		uint16_t y,
		uint16_t width,
		uint16_t height,
		WNDPROC windowProc,
		HINSTANCE hInstance);

	HWND GetHWND() const;

	SIZE GetSize() const;

	void ChangeVisibility(bool show);

private:
	HWND m_windowHwnd;

	POINT m_pos;
	SIZE m_size;

	bool m_isVisible;
};
