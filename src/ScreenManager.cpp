#include <cassert>
#include "ScreenManager.h"

ScreenManager ScreenManager::instance;

void ScreenManager::CreateHWND(WCHAR windowClass[], WCHAR title[], HINSTANCE hInstance)
{
	mHwnd = CreateWindowExW(0, windowClass, title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,
		nullptr, nullptr, hInstance, nullptr);
	assert(mHwnd != nullptr);
}