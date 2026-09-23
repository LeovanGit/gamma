#pragma once

#include "types.h"

class ScreenCapturer
{
public:
	ScreenCapturer(ScreenRect& rect);

    ~ScreenCapturer();

	void TakeScreenshot();

    void ApplyGamma();

    HDC GetScreenshot();

private:
    ScreenRect m_captureArea;

    HWND m_entireScreenHWND;
    HDC m_entireScreenHDC;
    HDC m_entireScreenMemHDC;

    void* m_pixels;
    HBITMAP m_hBitmap;
    HGDIOBJ m_hOldObject;
};
