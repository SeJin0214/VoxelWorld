#pragma once

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

class Timer;

struct DebugUiContext
{
	Timer& Timer;
};

class DebugUI
{
public:
	DebugUI() = default;
	~DebugUI() = default;
	DebugUI(const DebugUI& other) = delete;
	DebugUI& operator=(const DebugUI& rhs) = delete;

	void Draw(DebugUiContext& debugUiContext);
};