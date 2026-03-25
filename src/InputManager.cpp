#include <Windows.h>
#include "InputManager.h"
#include "Logger.h"
#include "ScreenManager.h"

InputManager::InputManager()
	: mKeyboard(std::make_unique<Keyboard>())
	, mMouse(std::make_unique<Mouse>())
	, mKeyboardTracker{}
	, mMouseTracker{}
	, mbIsMoved(false)
	, mbIsLeftButtonDown(false)
	, mbShouldChangedSpeed(false)
{
	mMouse->SetWindow(ScreenManager::GetInstance().GetHWND());
	mMouse->SetMode(Mouse::MODE_RELATIVE);
}

bool InputManager::Update()
{
	mbIsMoved = false;
	mbIsLeftButtonDown = false;
	mbShouldChangedSpeed = false;

	mKeyboardMovement = Vector3::Zero;
	Keyboard::State state = mKeyboard->GetState();

	mKeyboardTracker.Update(state);

	if (state.Escape)
	{
		return false;
		//Logger::Log("Z 키 누르는 중 - 아래로 이동!\n");
	}

	if (state.W)
	{
		mKeyboardMovement.z += 1.f;
		//Logger::Log("W 키 누르는 중 - 앞으로 이동!");
	}

	if (state.S)
	{
		mKeyboardMovement.z -= 1.f;
		//Logger::Log("S 키 누르는 중 - 뒤로 이동!\n");
	}

	if (state.A)
	{
		mKeyboardMovement.x -= 1.f;
		//Logger::Log("A 키 누르는 중 - 왼쪽으로 이동!\n");
	}

	if (state.D)
	{
		mKeyboardMovement.x += 1.f;
		//Logger::Log("D 키 누르는 중 - 오른쪽으로 이동!\n");
	}

	if (state.Q)
	{
		mKeyboardMovement.y += 1.f;
		//Logger::Log("Q 키 누르는 중 - 위로 이동!\n");
	}

	if (state.Z)
	{
		mKeyboardMovement.y -= 1.f;
		//Logger::Log("Z 키 누르는 중 - 아래로 이동!\n");
	}

	if (mKeyboardTracker.pressed.LeftShift)
	{
		mbShouldChangedSpeed = true;
	}

	if (mKeyboardMovement != Vector3::Zero)
	{
		mbIsMoved = true;
		mKeyboardMovement.Normalize();
	}

	mMouseMovement = Vector3::Zero;
	Mouse::State mouseState = mMouse->GetState();
	
	mMouseMovement.x = static_cast<float>(mouseState.x);
	mMouseMovement.y = static_cast<float>(mouseState.y);
	mMouseMovement.z = 0.0f; // 휠 값 등을 저장할 수도 있음

	if (mMouseMovement != Vector3::Zero)
	{
		mbIsMoved = true;
	}

	//Logger::Log("마우스 이동 x: %.2f, y: %.2f", mMouseMovement.x, mMouseMovement.y);
	mMouseTracker.Update(mouseState);
	if (mMouseTracker.leftButton == Mouse::ButtonStateTracker::PRESSED) 
	{
		LOG(LogSink::Console, LogLevel::Info, "마우스 클릭");
		mbIsLeftButtonDown = true;
	}
	return true;
}
