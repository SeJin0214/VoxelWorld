#include <cassert>
#include "ScreenManager.h"

ScreenManager ScreenManager::instance;

void ScreenManager::CreateHWND(WCHAR windowClass[], WCHAR title[], HINSTANCE hInstance)
{
	uint32_t screenWidth = GetSystemMetrics(SM_CXSCREEN);
	uint32_t screenHeight = GetSystemMetrics(SM_CYSCREEN);
	mHwnd = CreateWindowExW(0, windowClass, title, WS_POPUP | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, screenWidth, screenHeight,
		nullptr, nullptr, hInstance, nullptr);
	assert(mHwnd != nullptr);
}