#pragma once

#pragma comment(lib, "d3d11")

#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class DeviceFactory
{
public:
	struct DeviceBundle
	{
		ComPtr<IDXGISwapChain> SwapChain;
		ComPtr<ID3D11Device> Device;
		ComPtr<ID3D11DeviceContext> DeviceContext;
		D3D11_VIEWPORT Viewport{};
	};

	static DeviceBundle CreateDeviceAndSwapChain(HWND hWnd);
};
