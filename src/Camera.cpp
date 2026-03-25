#include <algorithm>
#include <string>
#include <DirectXMath.h>
#include "Camera.h"
#include "ScreenManager.h"
#include "MapManager.h"
#include "Logger.h"
#include "WorldConfig.h"
#include "InputManager.h"

Camera::Camera(const Vector3 position, const Vector3 rotation)
	: mPosition(position)
	, mRotation(rotation)
	, mbTransformDirty(false)
	, mSpeedLevel(0)
{
	CreatePjoectionMatrix();
	CreateViewMatrix(Vector3(), Vector3(), 0.0f);
}

void Camera::Update(const InputManager& inputManager, const float deltaTime, MapManager& mapManager)
{
	CameraInput input
	{
		input.KeyboardMovement = inputManager.GetKeyboardMovement(),
		input.MouseMovement = inputManager.GetMouseMovement(),
		input.bIsLeftButtonDown = inputManager.IsLeftButtonDown(),
		input.bShouldChangedSpeed = inputManager.ShouldChangedSpeed(),
	};
	Update(input, deltaTime, mapManager);
}

void Camera::Update(const CameraInput& input, const float deltaTime, MapManager& mapManager)
{
	Vector3 position = input.KeyboardMovement;
	Vector3 mouseMovement = input.MouseMovement;

	mbTransformDirty = true;
	if (position != Vector3::Zero || mouseMovement != Vector3::Zero)
	{
		mbTransformDirty = false;
		CreateViewMatrix(position, mouseMovement, deltaTime);
	}

	if (input.bIsLeftButtonDown)
	{
		TryRemoveBlock(mapManager);
	}

	if (input.bShouldChangedSpeed)
	{
		mSpeedLevel = (mSpeedLevel + 1) % MAX_LEVEL;
	}
}

void Camera::TryRemoveBlock(MapManager& mapManager) const
{
	Vector3 forward = GetForwardDirection();
	constexpr float STEP = 0.1f;
	float o = 0.5f;
	while (o < WorldConfig::RANGE)
	{
		Vector3 checkPos = mPosition + forward * o;
		o += STEP;

		LOG(LogSink::Console, LogLevel::Debug, "블록 제거 Try at (%f, %f, %f)", checkPos.x, checkPos.y, checkPos.z);
		if (mapManager.IsBlockAt(checkPos))
		{
			mapManager.RemoveBlockAt(checkPos);
			LOG(LogSink::Console, LogLevel::Debug, "블록 제거 at (%f, %f, %f)", checkPos.x, checkPos.y, checkPos.z);
			break;
		}
	}
}

void Camera::CreateViewMatrix(const Vector3 position, const Vector3 mouseMovement, const float deltaTime)
{
	constexpr float SENSITIVITY = 5.f;
	mRotation.y += mouseMovement.x * SENSITIVITY * deltaTime; // yaw 
	mRotation.x = std::clamp(mRotation.x + mouseMovement.y * SENSITIVITY * deltaTime, -89.9f, 89.9f); // pitch

	Vector3 rotationRad = Vector3(
		XMConvertToRadians(mRotation.x),
		XMConvertToRadians(mRotation.y),
		XMConvertToRadians(mRotation.z)
	);

	Quaternion q = Quaternion::CreateFromYawPitchRoll(rotationRad);
	mBasis = Matrix::CreateFromQuaternion(q);

	const float SPEED = GetCurrentSpeed();
	mPosition += position.x * mBasis.Right() * SPEED * deltaTime;
	mPosition += position.y * mBasis.Up() * SPEED * deltaTime;
	mPosition += position.z * GetForwardDirection() * SPEED * deltaTime;

	Matrix world = mBasis * Matrix::CreateTranslation(mPosition);
	mViewMatrix = world.Invert();

	mViewProjMatrix = mViewMatrix * mProjMatrix;
}

void Camera::CreatePjoectionMatrix()
{
	constexpr float FOV_RADIAN = DirectX::XMConvertToRadians(WorldConfig::FOV_DEGREES);
	const float ASPECT_RATIO = ScreenManager::GetInstance().GetClientAreaAspectRatio();

	mProjMatrix = DirectX::XMMatrixPerspectiveFovLH(FOV_RADIAN, ASPECT_RATIO, WorldConfig::NEAR_Z, WorldConfig::FAR_Z);
}

float Camera::GetCurrentSpeed() const
{
	assert(0 <= mSpeedLevel && mSpeedLevel < MAX_LEVEL);
	return SPEEDS[mSpeedLevel];
}




