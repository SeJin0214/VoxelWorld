#include "InputManager.h"
#include <windows.h>

InputManager::InputManager()
	: mKeyboard(std::make_unique<Keyboard>())
	, mTracker{}
{

}

Vector3 InputManager::GetMovementVector()
{
	Vector3 v = Vector3::Zero;
	Keyboard::State state = mKeyboard->GetState();


	mTracker.Update(state);

	if (state.W)
	{
		v.z += 1.f;
		OutputDebugStringA("W 키 누르는 중 - 앞으로 이동!\n");
	}

	if (state.S)
	{
		v.z -= 1.f;
		OutputDebugStringA("S 키 누르는 중 - 뒤로 이동!\n");
	}

	if (state.A)
	{
		v.x -= 1.f;
		OutputDebugStringA("A 키 누르는 중 - 왼쪽으로 이동!\n");
	}

	if (state.D)
	{
		v.x += 1.f;
		OutputDebugStringA("D 키 누르는 중 - 오른쪽으로 이동!\n");
	}

	if (v != Vector3::Zero)
	{
		v.Normalize();
	}

	return v;
}