#include "window.h"

Window::Window(
    uint16_t x,
    uint16_t y,
	uint16_t width,
	uint16_t height,
    WNDPROC windowProc,
    HINSTANCE hInstance)
{
    m_pos.x = x;
    m_pos.y = y;

    m_size.cx = width;
    m_size.cy = height;

    m_isVisible = true;

    WNDCLASSEX windowClass;
    ZeroMemory(&windowClass, sizeof(WNDCLASSEX));
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = 0;
    windowClass.lpszClassName = L"WindowClass1";
    RegisterClassEx(&windowClass);

    m_windowHwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        L"WindowClass1",
        L"gamma",
        WS_POPUP,
        x,
        y,
        m_size.cx,
        m_size.cy,
        NULL,
        NULL,
        hInstance,
        nullptr);

    // Disable ability to capture this window (so our ScreenCapturer doesn't capture it):
    SetWindowDisplayAffinity(m_windowHwnd, WDA_EXCLUDEFROMCAPTURE);

    ShowWindow(m_windowHwnd, SW_SHOW);
}

HWND Window::GetHWND() const
{
    return m_windowHwnd;
}

SIZE Window::GetSize() const
{
    return m_size;
}

void Window::ChangeVisibility(bool show)
{
    ShowWindow(m_windowHwnd, show ? SW_SHOW : SW_HIDE);
    m_isVisible = show;
}

void Window::ToggleVisibility()
{
    m_isVisible ? ChangeVisibility(false) : ChangeVisibility(true);
}

