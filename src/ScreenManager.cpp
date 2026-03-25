#include <cassert>
#include "ScreenManager.h"

ScreenManager ScreenManager::instance;

bool ScreenManager::CreateGlfwWindow(const char* name)
{
	// 주 모니터 갖고 오기
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	if (monitor == nullptr)
	{
		return false;
	}
	// 해상도 갖고 오기
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	if (mode == nullptr)
	{
		return false;
	}

	mWindow = glfwCreateWindow(mode->width, mode->height, name, monitor, nullptr);
	if (mWindow == nullptr)
	{
		return false;
	}
	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(mWindow, &width, &height);
	SetClientSize(width, height);
	return true;;
}

void ScreenManager::DestoryGlfwWindow()
{
	assert(mWindow != nullptr);
	glfwDestroyWindow(mWindow);
}