#pragma once

#include "types.h"

#include "window.h"

namespace
{
    constexpr uint8_t swapchainBuffersCount = 2;
} // namespace

class ScreenCapturer
{
public:
	ScreenCapturer(
        ScreenRect& rect,
        Window& window);

    ~ScreenCapturer();

    void Render();



	void TakeScreenshot();

    void ApplyGamma();

    HDC GetScreenshot();

private:
    void InitD3D12();
    void InitDeviceAndDebug();
    void InitCommandObjects();
    void InitSyncObjects();
    void InitDescriptorHeaps();
    void InitSwapchain();

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleRTV(uint32_t index);

    ScreenRect m_captureArea;

    HWND m_entireScreenHWND;
    HDC m_entireScreenHDC;
    HDC m_entireScreenMemHDC;

    void* m_pixels;
    HBITMAP m_hBitmap;
    HGDIOBJ m_hOldObject;




    Window& m_window;

    ComPtr<IDXGIFactory6> m_dxgiFactory;

    ComPtr<ID3D12Device> m_device;
    
#if defined(DEBUG) || defined(_DEBUG)
    ComPtr<ID3D12Debug1> m_debug;
#endif

    ComPtr<ID3D12CommandQueue> m_cmdQueue;
    ComPtr<ID3D12CommandAllocator> m_cmdAlloc;
    ComPtr<ID3D12GraphicsCommandList> m_cmdList;

    ComPtr<ID3D12Fence> m_fence;
    uint64_t m_fenceValue;

    ComPtr<ID3D12DescriptorHeap> m_descHeapRTV;
    // ComPtr<ID3D12DescriptorHeap> m_SRVDescHeap; // ?, for screnshot
    uint64_t m_descHandleIncrementSizeRTV;


    ComPtr<IDXGISwapChain1> m_swapchain;
    ComPtr<ID3D12Resource> m_swapchainBuffers[swapchainBuffersCount];
};
