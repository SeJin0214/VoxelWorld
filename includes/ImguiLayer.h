#pragma once

#include <windows.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

struct ImGuiLayerDesc
{
	HWND HWnd;
	ID3D11Device* Device;
	ID3D11DeviceContext* DeviceContext;
};

class ImGuiLayer
{
public:
	ImGuiLayer(ImGuiLayerDesc& layerDesc);
	~ImGuiLayer();
	ImGuiLayer(const ImGuiLayer& other) = delete;
	ImGuiLayer& operator=(const ImGuiLayer& rhs) = delete;

	void BeginFrame();
	void Render();

};