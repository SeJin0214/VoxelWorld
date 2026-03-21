#pragma once

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

class Profiler;
class RuntimeConfig;
class MapManager;
class Renderer;

struct DebugUIContext
{
	const RuntimeConfig& RuntimeConfig;
	const MapManager& MapManager;
	const Renderer& Renderer;
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
