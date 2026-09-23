#pragma once

#include <windows.h>

#include <d3d11.h>
#include <dxgi1_5.h>
#include <wrl/client.h>

#include <cstdint>
#include <vector>
#include <cassert>
#include <string>
#include <iostream>

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
