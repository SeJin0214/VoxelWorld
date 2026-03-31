#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class Profiler;
class RuntimeConfig;
class MapManager;
class Renderer;
class Camera;

struct DebugUIContext
{
	const Camera& mCamera;
	const RuntimeConfig& mRuntimeConfig;
	const MapManager& mMapManager;
	const Renderer& mRenderer;
};

class DebugUI
{
public:
	DebugUI() = default;
	~DebugUI() = default;
	DebugUI(const DebugUI& other) = delete;
	DebugUI& operator=(const DebugUI& rhs) = delete;

	void Draw(Profiler& profiler, const DebugUIContext& context);
};
