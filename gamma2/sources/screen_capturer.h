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
	ScreenCapturer(Window& window);

    ~ScreenCapturer();

    void Render();

	void TakeScreenshot();

    void ApplyGamma();

private:
    void InitD3D11();
    void InitDeviceAndDebug();
    void InitSwapchain();
    void InitDesktopDuplication();
    void InitSamplers();

    enum class ShaderType
    {
        VertexShader,
        PixelShader
    };

    void CompileShader(
        const std::wstring& filename,
        const std::string& entryPoint,
        ShaderType shaderType);

    Window& m_window;

    ComPtr<IDXGIFactory5> m_dxgiFactory;

    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_deviceContext;
    
#if defined(DEBUG) || defined(_DEBUG)
    ComPtr<ID3D11Debug> m_debug;
#endif

    ComPtr<IDXGISwapChain1> m_swapchain;
    ComPtr<ID3D11Texture2D> m_backbuffer;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    
    D3D11_VIEWPORT viewport;

    ComPtr<IDXGIOutputDuplication> m_desktopDuplication;
    ComPtr<ID3D11Texture2D> m_entireScreenImage;
    ComPtr<ID3D11ShaderResourceView> m_entireScreenImageSRV;
    ComPtr<ID3D11SamplerState> m_sampler;

    // Since we will have only one shader - I won't
    // create ShaderManager class:
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
};
