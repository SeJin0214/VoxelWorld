#include "ImGuiLayer.h"

ImGuiLayer::ImGuiLayer(ImGuiLayerDesc& layerDesc)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	// 넣기
	ImGui_ImplOpenGL3_Init( );
	ImGui_ImplGlfw_InitForOpenGL(layerDesc.Window, )
}

ImGuiLayer::~ImGuiLayer()
{
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiLayer::BeginFrame()
{
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
}

void ImGuiLayer::Render()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}