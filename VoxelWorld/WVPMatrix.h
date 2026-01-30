#pragma once
#include <DirectXMath.h>
using namespace DirectX;

struct WVPMatrix
{
    XMFLOAT4X4 WorldViewProj; // 64 bytes
};