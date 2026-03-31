#include <string>
#include "DebugUI.h"
#include "Profiler.h"
#include "RuntimeConfig.h"
#include "MapManager.h"
#include "Renderer.h"

void DebugUI::Draw(Profiler& profiler, const DebugUIContext& context)
{
	ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.15f);

	ImGui::Begin("Performance", nullptr,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize);
	Vector3 position = context.Camera.GetPosition();
	ImGui::Text("Position (%.2f, %.2f, %.2f)\n", position.x, position.y, position.z);
	ImGui::Text("FrameTimeMS %f\n", profiler.GetFrameTimeMS());
	ImGui::Text("FPS %u\n", profiler.GetFPS());

#ifdef PROFILE
	ImGui::Text("Frametime 5sec Average %u\n", profiler.Get5SecondAverageFPS());
	ImGui::Text("Frametime 15sec Average %u\n", profiler.Get15SecondAverageFPS());
	ImGui::Text("Render Distance %u\n", context.RuntimeConfig.GetRenderDistance());
	ImGui::Text("Loaded Chunks %u\n", context.MapManager.GetUsedChunkCount());
	ImGui::Text("Built Meshes %u\n", context.Renderer.GetBuiltMeshCount());
	ImGui::Text("Draw Meshs %u\n", context.Renderer.GetDrawMeshs());
	ImGui::Text("Pending Jobs %u\n", context.Renderer.GetPendingUploadsCount());


	ImGui::Separator();
	ImGui::Separator();

	const map<uint32_t, uint32_t>& buckets = profiler.GetFPSBucketCounts();

	uint32_t totalFrames = 0;
	for (const auto& pair : buckets)
	{
		totalFrames += pair.second;
	}

	ImGui::Text("Current: %u FPS", profiler.GetFPS());

	int slot = 120;
	while (slot >= 0)
	{
		uint32_t count = 0;
		auto iter = buckets.find(static_cast<uint32_t>(slot));
		if (iter != buckets.end())
		{
			count = iter->second;
		}

		float ratio = totalFrames > 0 ? static_cast<float>(count) / totalFrames : 0.0f;
		int barWidth = static_cast<int>(ratio * 25.0f);

		char label[64];
		if (slot == 120)
		{
			sprintf_s(label, "120+");
			slot = 110;
		}
		else if (slot > 30)
		{
			sprintf_s(label, "%3d-%3d", slot, slot + 9);
			slot -= 10;
		}
		else if (slot == 30)
		{
			sprintf_s(label, "%3d-%3d", slot, slot + 9);
			slot -= 5;
		}
		else
		{
			sprintf_s(label, "%2d-%2d*", slot, slot + 4);
			slot -= 5;
		}

		std::string bar(barWidth, ')');
		bar.append(25 - barWidth, ' ');

		ImGui::Text("%-7s : %s %u f (%.0f%%)",
			label,
			bar.c_str(),
			count,
			ratio * 100.0f);

		if (slot == 25)
		{
			ImGui::Separator();
			ImGui::Text("[ CRITICAL BOUNDARY (30) ]");
			ImGui::Separator();
		}
	}

#else
	static_cast<void>(context);
#endif

	ImGui::End();
}
