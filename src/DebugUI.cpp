#include "DebugUI.h"
#include "Timer.h"

void DebugUI::Draw(DebugUiContext& debugUiContext)
{
	Timer& timer = debugUiContext.Timer;

	ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.15f);

	ImGui::Begin("Performance", nullptr,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("FPS: %d", timer.GetFPS());
	ImGui::Text("Frame Time: %.2f ms", timer.GetDeltaTime() * 1000.0f);




	ImGui::End();
}