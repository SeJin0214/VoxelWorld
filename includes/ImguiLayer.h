#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

struct ImguiLayerDesc
{
	GLFWwindow* Window;
	const char* Version;
};

class ImguiLayer
{
public:
	ImguiLayer(ImguiLayerDesc& layerDesc);
	~ImguiLayer();
	ImguiLayer(const ImguiLayer& other) = delete;
	ImguiLayer& operator=(const ImguiLayer& rhs) = delete;

	void BeginFrame();
	void Render();

};