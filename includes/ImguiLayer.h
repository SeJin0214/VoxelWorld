#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

struct ImGuiLayerDesc
{
	GLFWwindow* Window;
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