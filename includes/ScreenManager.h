#pragma once
#include <windows.h>
#include "Types.h"

class ScreenManager
{
public:
	static ScreenManager& GetInstance() { return instance; }
	static uint32_t GetWindowWidth() { return WIDTH; }
	static uint32_t GetWindowHeight() { return HEIGHT; }

	void CreateHWND(WCHAR windowClass[], WCHAR title[], HINSTANCE hInstance);
	HWND GetHWND() const { return mHwnd; }

	float GetClientAreaAspectRatio() const { return static_cast<float>(mClientWidth) / mClientHeight; }
	uint32_t GetClientWidth() const { return mClientWidth; }
	uint32_t GetClientHeight() const { return mClientHeight; }
	void SetClientSize(const uint32_t width, const uint32_t height) { mClientWidth = width; mClientHeight = height; }

private:
	static ScreenManager instance;
	static const uint32_t WIDTH = 1680;
	static const uint32_t HEIGHT = 1024;

	HWND mHwnd;
	uint32_t mClientWidth;
	uint32_t mClientHeight;

	ScreenManager() = default;
	~ScreenManager() = default;
	ScreenManager(const ScreenManager& other) = delete;
	ScreenManager& operator=(const ScreenManager& rhs) = delete;
};