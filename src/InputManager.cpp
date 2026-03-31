#include "InputManager.h"
#include "Logger.h"
#include "ScreenManager.h"

InputManager::InputManager(GLFWwindow* window)
	: mbIsMoved(false)
	, mbIsLeftButtonDown(false)
	, mbShouldChangedSpeed(false)
	, mbIsPrevPressed(false)
{
	glfwGetCursorPos(window, &mPrevMouseX, &mPrevMouseY);
}

bool InputManager::Update(GLFWwindow* window)
{
	mbIsMoved = false;
	mbIsLeftButtonDown = false;
	mbShouldChangedSpeed = false;

	mKeyboardMovement = Vector3(0.f, 0.f, 0.f);

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		return false;
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		mKeyboardMovement.z += 1.f;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		mKeyboardMovement.z -= 1.f;
		//Logger::Log("S 키 입력 시 - 뒤로 이동!\n");
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		mKeyboardMovement.x -= 1.f;
		//Logger::Log("A 키 입력 시 - 왼쪽으로 이동!\n");
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		mKeyboardMovement.x += 1.f;
		//Logger::Log("D 키 입력 시 - 오른쪽으로 이동!\n");
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		mKeyboardMovement.y += 1.f;
		//Logger::Log("Q 키 입력 시 - 위로 이동!\n");
	}

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		mKeyboardMovement.y -= 1.f;
		//Logger::Log("Z 키 입력 시 - 아래로 이동!\n");
	}

	// Press로 변경하기
	bool bIsPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
	if (bIsPressed && mbIsPrevPressed == false)
	{
		mbShouldChangedSpeed = true;
	}
	mbIsPrevPressed = bIsPressed;

	if (glm::length(mKeyboardMovement) > 0.f)
	{
		mbIsMoved = true;
		mKeyboardMovement = glm::normalize(mKeyboardMovement);
	}

	double mouseX = 0.0;
	double mouseY = 0.0;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	
	mMouseMovement.x = static_cast<float>(mouseX - mPrevMouseX);
	mMouseMovement.y = static_cast<float>(mouseY - mPrevMouseY);
	mPrevMouseX = mouseX;
	mPrevMouseY = mouseY;

	if (glm::length(mMouseMovement) > 0.f)
	{
		mbIsMoved = true;
	}

	//Logger::Log("마우스 이동 x: %.2f, y: %.2f", mMouseMovement.x, mMouseMovement.y);
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		LOG(LogSink::Console, LogLevel::Info, "마우스 클릭");
		mbIsLeftButtonDown = true;
	}
	return true;
}
