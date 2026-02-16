#pragma once
#include <Keyboard.h>
#include <Mouse.h>
#include <memory>
#include <SimpleMath.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

class InputManager
{

public:
	InputManager();
	InputManager(const InputManager& other) = delete;
	InputManager& operator=(const InputManager& rhs) = delete;
	~InputManager() = default;

	bool Update();
	Vector3 GetMouseMovement() const { return mMouseMovement; }
	Vector3 GetKeyboardMovement() const { return mKeyboardMovement; }
	bool IsMoved() const { return mbIsMoved; }
	bool IsLeftButtonDown() const { return mbIsLeftButtonDown; }

private:
	std::unique_ptr<Keyboard> mKeyboard;
	std::unique_ptr<Mouse> mMouse;
	Keyboard::KeyboardStateTracker mKeyboardTracker;
	Mouse::ButtonStateTracker mMouseTracker;
	Vector3 mKeyboardMovement;
	Vector3 mMouseMovement;
	
	bool mbIsMoved;
	bool mbIsLeftButtonDown;
};