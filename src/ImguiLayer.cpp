#include "ImguiLayer.h"

ImguiLayer::ImguiLayer(ImguiLayerDesc& layerDesc)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	// 넣기
	ImGui_ImplOpenGL3_Init(layerDesc.Version);
	// true로 해야 GLFW 키 입력을 Imgui가 자동으로 인식함
	ImGui_ImplGlfw_InitForOpenGL(layerDesc.Window, true);
}

ImguiLayer::~ImguiLayer()
{
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();
}

void ImguiLayer::BeginFrame()
{
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
}

void ImguiLayer::Render()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}