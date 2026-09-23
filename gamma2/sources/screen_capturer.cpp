#include "screen_capturer.h"

ScreenCapturer::ScreenCapturer(ScreenRect& rect) : m_captureArea(rect)
{
    // Window's Handle (pointer to specific window):
    m_entireScreenHWND = GetDesktopWindow();

    // Device Context Handle (pointer to pixels buffer of this
    // window (bitmap) and interface for drawing into it):
    m_entireScreenHDC = GetDC(m_entireScreenHWND);

    // Memory DC (pointer to copy of window's Device Context in memory,
    // so we can draw into it and then copy into real DC, like swapchain):
    m_entireScreenMemHDC = CreateCompatibleDC(m_entireScreenHDC);

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = m_captureArea.width;
    bmi.bmiHeader.biHeight = -m_captureArea.height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; // BGRA (8 bits per channel)
    bmi.bmiHeader.biCompression = BI_RGB;

    // Create Bitmap Handle (pixels buffer), allocate memory for it into m_pixels ptr:
    m_hBitmap = CreateDIBSection(m_entireScreenHDC, &bmi, DIB_RGB_COLORS, &m_pixels, NULL, 0);

    // Bind m_hBitmap to Memory DC (m_entireScreenMemHDC)
    // and save prev. bind into m_hOldObject (for destructor):
    m_hOldObject = SelectObject(m_entireScreenMemHDC, m_hBitmap);
}

ScreenCapturer::~ScreenCapturer()
{
    // Extract back m_hBitmap from m_entireScreenMemHDC
    // (we need to do this before delete):
    SelectObject(m_entireScreenMemHDC, m_hOldObject);

    DeleteObject(m_hBitmap);
    DeleteDC(m_entireScreenMemHDC);
    ReleaseDC(m_entireScreenHWND, m_entireScreenHDC);
}

void ScreenCapturer::TakeScreenshot()
{
    // Copy real screen pixels (from m_entireScreenHDC's bitmap)
    // to memory (m_entireScreenMemHDC's m_hBitmap).
    // Copy from VRAM to RAM.
    // After this m_pixels ptr stores raw pixels data:
    BitBlt( // StretchBlt faster, Windows Graphics Capture (WGC) + DirectX more faster (CPU vs GPU)
        m_entireScreenMemHDC,
        0,
        0,
        m_captureArea.width,
        m_captureArea.height,
        m_entireScreenHDC,
        m_captureArea.left,
        m_captureArea.top,
        SRCCOPY);
}

void ScreenCapturer::ApplyGamma()
{
    RGBQUAD* pixels = static_cast<RGBQUAD*>(m_pixels);

    uint32_t pixelsCount = m_captureArea.width * m_captureArea.height;

    for (uint32_t i = 0; i != pixelsCount; ++i)
    {
        BYTE gray = static_cast<BYTE>(
            pixels[i].rgbRed * 0.299 +
            pixels[i].rgbGreen * 0.587 +
            pixels[i].rgbBlue * 0.114);

        pixels[i].rgbRed = gray;
        pixels[i].rgbGreen = gray;
        pixels[i].rgbBlue = gray;
        //pixels[i].rgbReserved = 255; // alpha
    }
}

HDC ScreenCapturer::GetScreenshot()
{
    return m_entireScreenMemHDC;
}
