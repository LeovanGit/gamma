#pragma once

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <cstdint>
#include <vector>
#include <cassert>

struct ScreenRect
{
    uint16_t left;
    uint16_t top;
    uint16_t width;
    uint16_t height;
};

template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

#define Ensure(D3DFunc)              \
do {                                 \
    HRESULT result;                  \
    result = (D3DFunc);              \
    assert(result >= 0 && #D3DFunc); \
} while (false);
