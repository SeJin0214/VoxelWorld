#pragma once
#include <GLFW/glfw3.h>
#include "Types.h"

class InputManager
{
public:
	InputManager(GLFWwindow* window);
	InputManager(const InputManager& other) = delete;
	InputManager& operator=(const InputManager& rhs) = delete;
	~InputManager() = default;

	bool Update(GLFWwindow* window);
	Vector3 GetKeyboardMovement() const { return mKeyboardMovement; }
	Vector2 GetMouseMovement() const { return mMouseMovement; }
	bool IsMoved() const { return mbIsMoved; }
	bool IsLeftButtonDown() const { return mbIsLeftButtonDown; }
	bool ShouldChangedSpeed() const { return mbShouldChangedSpeed; }

private:
	Vector3 mKeyboardMovement;
	Vector2 mMouseMovement;
	double mPrevMouseX;
	double mPrevMouseY;
	
	bool mbIsMoved;
	bool mbIsLeftButtonDown;
	bool mbShouldChangedSpeed;
	bool mbIsPrevPressed;
};