#pragma once
#include "Camera.h"
#include "Types.h"

class TestRunner
{
public:
	TestRunner(Vector3 position)
		: mPosition(position)
		, mYawRotate(0.f)
		, mStartTime(0.f)
	{

	}

	CameraInput Update(const float deltaTime)
	{
		CameraInput result{};
		if (mStartTime == 0.f)
		{
			result.bShouldChangedSpeed = true;
		}
		mStartTime += deltaTime;
		if (mStartTime < 10.f)
		{
			result.KeyboardMovement = Vector3(0.f, -0.1f, 1.f);
		}
		else if (mStartTime < 100.f)
		{
			if (mYawRotate <= 90)
			{
				constexpr float SPEED = 5.f;
				result.MouseMovement = Vector3(1.f, 0.f, 0.f);
				mYawRotate += deltaTime * SPEED;
			}
			result.bIsLeftButtonDown = true;
			result.KeyboardMovement = Vector3(0.f, 0.f, 1.f);
		}
		else if (mStartTime < 200.f)
		{
			if (mYawRotate > 0)
			{
				constexpr float SPEED = 5.f;
				result.MouseMovement = Vector3(-1.f, 0.f, 0.f);
				mYawRotate -= deltaTime * SPEED;
			}
			result.KeyboardMovement = Vector3(0.f, 0.f, 1.f);
		}
		else if (mStartTime < 300.f)
		{
			result.KeyboardMovement = Vector3(1.f, 0.f, 1.f);
		}
		else if (mStartTime < 1000.f)
		{
			result.KeyboardMovement = Vector3(0.f, 0.f, 1.f);
		}
		else if (mStartTime < 1010.f)
		{
			result.KeyboardMovement = Vector3(0.f, 0.1f, 1.f);

		}
		else if (mStartTime < 1050.f)
		{
			if (mYawRotate <= 180)
			{
				constexpr float SPEED = 5.f;
				result.MouseMovement = Vector3(1.f, 0.f, 0.f);
				mYawRotate -= deltaTime * SPEED;
			}
		}
		else
		{
			mStartTime = 0.f;
		}

		return result;
	}

	Vector3 GetPostion() const { return mPosition; }

private:
	Vector3 mPosition;
	float mYawRotate;
	float mStartTime;

};