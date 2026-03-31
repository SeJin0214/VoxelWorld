#include <algorithm>
#include "Camera.h"
#include "ScreenManager.h"
#include "MapManager.h"
#include "Logger.h"
#include "WorldConfig.h"
#include "InputManager.h"
#include "glm/gtx/transform.hpp" // 이동행렬
#include "glm/gtc/matrix_transform.hpp" // 투영행렬
#include "glm/gtc/quaternion.hpp"

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
	CameraInput input{};
	input.KeyboardMovement = inputManager.GetKeyboardMovement();
	input.MouseMovement = inputManager.GetMouseMovement();
	input.bIsLeftButtonDown = inputManager.IsLeftButtonDown();
	input.bShouldChangedSpeed = inputManager.ShouldChangedSpeed();
	Update(input, deltaTime, mapManager);
}

void Camera::Update(const CameraInput& input, const float deltaTime, MapManager& mapManager)
{
	Vector3 position = input.KeyboardMovement;
	Vector3 mouseMovement = Vector3(input.MouseMovement.x, input.MouseMovement.y, 0.f);

	mbTransformDirty = true;
	if (position != Vector3(0.f, 0.f, 0.f) || mouseMovement != Vector3(0.f, 0.f, 0.f))
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
	constexpr float SENSITIVITY = 0.5f;
	// Cursor delta is already measured per frame, so don't scale it by deltaTime.
	// Window Y grows downward, so pitch needs the opposite sign for FPS-style look.
	mRotation.y -= mouseMovement.x * SENSITIVITY; // yaw
	mRotation.x = std::clamp(mRotation.x - mouseMovement.y * SENSITIVITY, -89.9f, 89.9f); // pitch

	Vector3 rotationRad = Vector3(
		glm::radians(mRotation.x),
		glm::radians(mRotation.y),
		glm::radians(mRotation.z)
	);

	glm::quat q(rotationRad);
	mBasis = glm::mat4_cast(q);

	const float SPEED = GetCurrentSpeed();
	mPosition += position.x * GetRightDirection() * SPEED * deltaTime;
	mPosition += position.y * GetUpDirection() * SPEED * deltaTime;
	mPosition += position.z * GetForwardDirection() * SPEED * deltaTime;

	Matrix world = glm::translate(mPosition) * mBasis;
	mViewMatrix = glm::inverse(world);

	mViewProjMatrix =  mProjMatrix * mViewMatrix;
}

void Camera::CreatePjoectionMatrix()
{
	constexpr float FOV_RADIAN = glm::radians(WorldConfig::FOV_DEGREES);
	const float ASPECT_RATIO = ScreenManager::GetInstance().GetClientAreaAspectRatio();

	mProjMatrix = glm::perspective(FOV_RADIAN, ASPECT_RATIO, WorldConfig::NEAR_Z, WorldConfig::FAR_Z);
}

float Camera::GetCurrentSpeed() const
{
	assert(0 <= mSpeedLevel && mSpeedLevel < MAX_LEVEL);
	return SPEEDS[mSpeedLevel];
}





