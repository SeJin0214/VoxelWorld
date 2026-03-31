#include <cassert>
#include "ScreenManager.h"

ScreenManager ScreenManager::instance;

bool ScreenManager::CreateGlfwWindow(const char* name)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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
	SetWindowSize(static_cast<uint32_t>(mode->width), static_cast<uint32_t>(mode->height));
	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(mWindow, &width, &height);
	SetClientSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	return true;;
}

void ScreenManager::DestoryGlfwWindow()
{
	assert(mWindow != nullptr);
	glfwDestroyWindow(mWindow);
	mWindow = nullptr;
}