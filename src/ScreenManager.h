#pragma once
#include <windows.h>

class ScreenManager
{
public:
	static ScreenManager& GetInstance() { return instance; }
	static int GetWindowWidth() { return WIDTH; }
	static int GetWindowHeight() { return HEIGHT; }

	void CreateHWND(WCHAR windowClass[], WCHAR title[], HINSTANCE hInstance);

	HWND GetHWND() const { return mHwnd; }
	float GetClientAreaAspectRatio() const { return static_cast<float>(mClientWidth) / mClientHeight; }
	void SetClientSize(const int width, const int height) { mClientWidth = width; mClientHeight = height; }

private:
	static ScreenManager instance;
	static const int WIDTH = 1024;
	static const int HEIGHT = 1024;

	HWND mHwnd;
	int mClientWidth;
	int mClientHeight;

	ScreenManager() = default;
	~ScreenManager() = default;
	ScreenManager(const ScreenManager& other) = delete;
	ScreenManager& operator=(const ScreenManager& rhs) = delete;
};