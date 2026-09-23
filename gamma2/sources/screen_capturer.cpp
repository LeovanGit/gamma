#include "screen_capturer.h"

ScreenCapturer::ScreenCapturer(
    ScreenRect& rect,
    Window& window)
    : m_captureArea(rect)
    , m_window(window)
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

    
    

    InitD3D12();
}

void ScreenCapturer::InitD3D12()
{
    InitDeviceAndDebug();
    InitCommandObjects();
    InitSyncObjects();
    InitDescriptorHeaps();
    InitSwapchain();
}

void ScreenCapturer::InitDeviceAndDebug()
{
    Ensure(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

#if defined(DEBUG) || defined(_DEBUG)
    Ensure(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug)));

    m_debug->EnableDebugLayer();
    m_debug->SetEnableGPUBasedValidation(true);
#endif

    Ensure(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)));
}

void ScreenCapturer::InitCommandObjects()
{
    D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cmdQueueDesc.NodeMask = 0;

    Ensure(m_device->CreateCommandQueue(
        &cmdQueueDesc,
        IID_PPV_ARGS(&m_cmdQueue)));

    Ensure(m_device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&m_cmdAlloc)));

    Ensure(m_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_cmdAlloc.Get(),
        nullptr,
        IID_PPV_ARGS(&m_cmdList)));

    m_cmdList->Close();
}

void ScreenCapturer::InitSyncObjects()
{
    m_fenceValue = 0;
    Ensure(m_device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
}

void ScreenCapturer::InitDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDesc.NumDescriptors = swapchainBuffersCount;
    rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    rtvDesc.NodeMask = 0;

    Ensure(m_device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&m_descHeapRTV)));

    m_descHandleIncrementSizeRTV = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void ScreenCapturer::InitSwapchain()
{
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
    swapchainDesc.Width = m_window.GetSize().cx;
    swapchainDesc.Height = m_window.GetSize().cy;
    swapchainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapchainDesc.BufferCount = swapchainBuffersCount;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.Flags = 0;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapchainFullscreenDesc = {};
    swapchainFullscreenDesc.Windowed = true;

    Ensure(m_dxgiFactory->CreateSwapChainForHwnd(
        m_device.Get(),
        m_window.GetHWND(),
        &swapchainDesc,
        &swapchainFullscreenDesc,
        nullptr,
        &m_swapchain));

    for (uint8_t i = 0; i != swapchainBuffersCount; ++i)
    {
        Ensure(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_swapchainBuffers[i])));

        m_device->CreateRenderTargetView(m_swapchainBuffers[i].Get(), nullptr, GetCPUDescriptorHandleRTV(i));
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE ScreenCapturer::GetCPUDescriptorHandleRTV(uint32_t index)
{
    D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = m_descHeapRTV->GetCPUDescriptorHandleForHeapStart();
    heapHandle.ptr += index * m_descHandleIncrementSizeRTV;

    return heapHandle;
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


void ScreenCapturer::Render()
{

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
