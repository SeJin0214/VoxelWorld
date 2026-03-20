#include <cassert>
#include <iostream>
#include <dxgi1_6.h>

#include "DeviceFactory.h"
#include "ScreenManager.h"

namespace
{
	constexpr UINT NVIDIA_VENDOR_ID = 0x10DE;

	ComPtr<IDXGIAdapter1> FindPreferredAdapter()
	{
		ComPtr<IDXGIFactory6> factory;
		HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(factory.GetAddressOf()));
		if (FAILED(hr))
		{
			return nullptr;
		}

		ComPtr<IDXGIAdapter1> fallbackAdapter;
		for (UINT adapterIndex = 0; ; ++adapterIndex)
		{
			ComPtr<IDXGIAdapter1> adapter;
			hr = factory->EnumAdapterByGpuPreference(
				adapterIndex,
				DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
				IID_PPV_ARGS(adapter.GetAddressOf()));
			if (hr == DXGI_ERROR_NOT_FOUND)
			{
				break;
			}
			if (FAILED(hr))
			{
				continue;
			}

			DXGI_ADAPTER_DESC1 adapterDesc{};
			hr = adapter->GetDesc1(&adapterDesc);
			if (FAILED(hr))
			{
				continue;
			}

			printf("EnumAdapterByGpuPreference: %ws (VendorId=0x%X)\n",
				adapterDesc.Description, adapterDesc.VendorId);

			if (fallbackAdapter == nullptr)
			{
				fallbackAdapter = adapter;
			}

			if (adapterDesc.VendorId == NVIDIA_VENDOR_ID)
			{
				return adapter;
			}
		}

		return fallbackAdapter;
	}
}

DeviceBundle DeviceFactory::CreateDeviceAndSwapChain(HWND hWnd)
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

	ComPtr<IDXGIAdapter1> selectedAdapter = FindPreferredAdapter();
	if (selectedAdapter != nullptr)
	{
		DXGI_ADAPTER_DESC1 selectedAdapterDesc{};
		HRESULT selectedAdapterDescHr = selectedAdapter->GetDesc1(&selectedAdapterDesc);
#ifdef _DEBUG
		if (SUCCEEDED(selectedAdapterDescHr))
		{
			printf("Preferred DXGI Adapter: %ws\n", selectedAdapterDesc.Description);
		}
#endif
	}

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		selectedAdapter.Get(),
		selectedAdapter != nullptr ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
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


#ifdef _DEBUG
	ComPtr<IDXGIDevice> dxgiDevice;
	hr = bundle.Device.As(&dxgiDevice);
	assert(SUCCEEDED(hr));

	ComPtr<IDXGIAdapter> adapter;
	hr = dxgiDevice->GetAdapter(&adapter);
	assert(SUCCEEDED(hr));

	DXGI_ADAPTER_DESC adapterDesc{};
	hr = adapter->GetDesc(&adapterDesc);
	assert(SUCCEEDED(hr));

	printf("DXGI Adapter: %ws\n", adapterDesc.Description);
#endif

	return bundle;
}
