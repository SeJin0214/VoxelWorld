#pragma once

#include <Keyboard.h>
#include <memory>
#include <string>
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

	Vector3 GetMovementVector();

private:
	std::unique_ptr<Keyboard> mKeyboard;
	Keyboard::KeyboardStateTracker mTracker;
};