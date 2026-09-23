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

	void ChangeVisibility(bool show);

	void ShowImage(HDC image);

private:
	POINT m_pos;
	SIZE m_size;

	bool m_isVisible;

	HWND m_windowHwnd;

	BLENDFUNCTION m_blend;
};
