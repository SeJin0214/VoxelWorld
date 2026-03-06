#include <cassert>

#include "DeviceFactory.h"
#include "ScreenManager.h"

DeviceFactory::DeviceBundle DeviceFactory::CreateDeviceAndSwapChain(HWND hWnd)
{
	DeviceBundle bundle{};

	D3D_FEATURE_LEVEL featurelevels[] = { D3D_FEATURE_LEVEL_11_0 };

	DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
	swapchaindesc.BufferDesc.Width = 0;
	swapchaindesc.BufferDesc.Height = 0;
	swapchaindesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapchaindesc.SampleDesc.Count = 1;
	swapchaindesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchaindesc.BufferCount = 2;
	swapchaindesc.OutputWindow = hWnd;
	swapchaindesc.Windowed = TRUE;
	swapchaindesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	UINT flag = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	flag = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flag,
		featurelevels,
		ARRAYSIZE(featurelevels),
		D3D11_SDK_VERSION,
		&swapchaindesc,
		bundle.SwapChain.GetAddressOf(),
		bundle.Device.GetAddressOf(),
		nullptr,
		bundle.DeviceContext.GetAddressOf());

	assert(SUCCEEDED(hr));
	assert(bundle.SwapChain != nullptr);

	bundle.SwapChain->GetDesc(&swapchaindesc);

	const UINT width = swapchaindesc.BufferDesc.Width;
	const UINT height = swapchaindesc.BufferDesc.Height;

	ScreenManager::GetInstance().SetClientSize(width, height);
	bundle.Viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };

	return bundle;
}
