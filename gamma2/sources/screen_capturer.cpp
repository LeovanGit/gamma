#include "screen_capturer.h"

#include <d3dcompiler.h>
#include "DDSTextureLoader11.h"

// Use discrete GPU instead of integrated by default:
extern "C"
{
    _declspec(dllexport) uint32_t NvOptimusEnablement = 1;
    _declspec(dllexport) uint32_t AmdPowerXpressRequestHighPerformance = 1;
}

ScreenCapturer::ScreenCapturer(Window& window) : m_window(window)
{
    InitD3D11();

    CompileShader(L"shaders/gamma.hlsl", "FullscreenTriangleVS", ShaderType::VertexShader);
    CompileShader(L"shaders/gamma.hlsl", "GammaPS", ShaderType::PixelShader);

    // LoadTexture(L"assets/bunny.dds", m_bunnySRV);
    LoadTexture(L"assets/ui_mask.dds", m_uiMaskSRV);
}

void ScreenCapturer::InitD3D11()
{
    InitDeviceAndDebug();
    InitSwapchain();
    InitDesktopDuplication();
    InitSamplers();

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = m_window.GetSize().cx;
    viewport.Height = m_window.GetSize().cy;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
}

void ScreenCapturer::InitDeviceAndDebug()
{
    Ensure(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

    constexpr D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

#if defined(DEBUG) || defined(_DEBUG)
    UINT flags = D3D11_CREATE_DEVICE_DEBUG;
#else
    UINT flags = 0;
#endif

    Ensure(D3D11CreateDevice(
        nullptr, // use default GPU
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        &featureLevel,
        1,
        D3D11_SDK_VERSION,
        &m_device,
        nullptr,
        &m_deviceContext));

#if defined(DEBUG) || defined(_DEBUG)
    Ensure(m_device->QueryInterface(IID_PPV_ARGS(&m_debug)));
#endif
}

void ScreenCapturer::InitSwapchain()
{
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
    swapchainDesc.Width = m_window.GetSize().cx;
    swapchainDesc.Height = m_window.GetSize().cy;
    swapchainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapchainDesc.BufferCount = swapchainBuffersCount;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
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

    Ensure(m_swapchain->GetBuffer(0, IID_PPV_ARGS(&m_backbuffer)));

    Ensure(m_device->CreateRenderTargetView(
        m_backbuffer.Get(),
        nullptr,
        &m_renderTargetView));
}

void ScreenCapturer::InitDesktopDuplication()
{
    ComPtr<IDXGIDevice> dxgiDevice;
    Ensure(m_device->QueryInterface(IID_PPV_ARGS(&dxgiDevice)));

    // GPU (which was selected on D3D11Device creation):
    ComPtr<IDXGIAdapter> dxgiAdapter;
    Ensure(dxgiDevice->GetAdapter(&dxgiAdapter));

    // Main monitor (index 0):
    ComPtr<IDXGIOutput> dxgiOutput;
    Ensure(dxgiAdapter->EnumOutputs(0, &dxgiOutput));

    // IDXGIOutput1 interface supports DuplicateOutput():
    ComPtr<IDXGIOutput1> dxgiOutput1;
    Ensure(dxgiOutput->QueryInterface(IID_PPV_ARGS(&dxgiOutput1)));

    // Enable entire screen image duplication:
    Ensure(dxgiOutput1->DuplicateOutput(m_device.Get(), &m_desktopDuplication));
}

void ScreenCapturer::InitSamplers()
{
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

    Ensure(m_device->CreateSamplerState(&samplerDesc, &m_sampler));
}

void ScreenCapturer::CompileShader(
    const std::wstring& filename,
    const std::string& entryPoint,
    ShaderType shaderType)
{
#if defined(DEBUG) || defined(_DEBUG)
    uint32_t flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION; // for RenderDoc
#else
    uint32_t flags = 0;
#endif

    ComPtr<ID3DBlob> compiledBytecode;
    ComPtr<ID3DBlob> error;

    HRESULT result = D3DCompileFromFile(
        filename.c_str(),
        NULL,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint.c_str(),
        shaderType == ShaderType::VertexShader ? "vs_5_0" : "ps_5_0",
        flags,
        0,
        &compiledBytecode,
        &error);

    if (result < 0 && error)
    {
        char* errorString = static_cast<char*>(error->GetBufferPointer());
        assert(false && "Failed to compile shader\n");
    }

    if (shaderType == ShaderType::VertexShader)
    {
        Ensure(m_device->CreateVertexShader(
            compiledBytecode->GetBufferPointer(),
            compiledBytecode->GetBufferSize(),
            nullptr,
            &m_vertexShader));
    }
    else
    {
        Ensure(m_device->CreatePixelShader(
            compiledBytecode->GetBufferPointer(),
            compiledBytecode->GetBufferSize(),
            nullptr,
            &m_pixelShader));
    }
}

void ScreenCapturer::LoadTexture(
    const std::wstring& filename,
    ComPtr<ID3D11ShaderResourceView>& outTextureSRV)
{

    Ensure(DirectX::CreateDDSTextureFromFile(
        m_device.Get(),
        filename.c_str(),
        nullptr,
        &outTextureSRV));
}

ScreenCapturer::~ScreenCapturer()
{
#if defined(DEBUG) || defined(_DEBUG)
    m_debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif
}

void ScreenCapturer::Render()
{
    TakeScreenshot();

    ApplyGamma();

    m_swapchain->Present(0, 0);
}

void ScreenCapturer::TakeScreenshot()
{
    static bool isFirstFrame = true;

    if (!isFirstFrame)
    {
        // we need to release prev. screenshot before acquire new:
        m_desktopDuplication->ReleaseFrame();
    }

    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    ComPtr<IDXGIResource> screenshot;
    Ensure(m_desktopDuplication->AcquireNextFrame(INFINITE, &frameInfo, &screenshot));

    Ensure(screenshot->QueryInterface(IID_PPV_ARGS(&m_entireScreenImage)));

    // Create SRV for screenshot:
    if (isFirstFrame)
    {        
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // IDXGIOutputDuplication::AcquireNextFrame() returns BRGA8
        srvDesc.Texture2D.MipLevels = 1;

        Ensure(m_device->CreateShaderResourceView(
            m_entireScreenImage.Get(),
            &srvDesc,
            &m_entireScreenImageSRV));

        isFirstFrame = false;
    }
}

void ScreenCapturer::ApplyGamma()
{
    m_deviceContext->RSSetViewports(1, &viewport);

    m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

    m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    ID3D11ShaderResourceView* textures[] =
    {
        m_entireScreenImageSRV.Get(),
        m_uiMaskSRV.Get(),
        //m_bunnySRV.Get()
    };

    m_deviceContext->PSSetShaderResources(0, 2, textures);
    // m_deviceContext->PSSetSamplers(0, 1, m_sampler.GetAddressOf());

    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_deviceContext->Draw(3, 0);
}
