#include <algorithm>
#include <string>
#include <DirectXMath.h>
#include "Camera.h"
#include "ScreenManager.h"
#include "InputManager.h"
#include "MapManager.h"
#include "Logger.h"

Camera::Camera(const Vector3 position, const Vector3 rotation)
	: mPosition(position)
	, mRotation(rotation)
	, mRenderDistance(16)
	, mbTransformDirty(false)
{
	CreatePjoectionMatrix();
}

void Camera::Update(const InputManager& inputManager, const float deltaTime)
{
	Vector3 position = inputManager.GetKeyboardMovement();
	Vector3 mouseMovement = inputManager.GetMouseMovement();

	mbTransformDirty = true;
	if (position != Vector3::Zero || mouseMovement != Vector3::Zero)
	{
		mbTransformDirty = false;
		const float sensitivity = 5.f;
		mRotation.y += mouseMovement.x * sensitivity * deltaTime; // yaw 
		mRotation.x = std::clamp(mRotation.x + mouseMovement.y * sensitivity * deltaTime, -89.9f, 89.9f); // pitch

		Vector3 rotationRad = Vector3(
			XMConvertToRadians(mRotation.x),
			XMConvertToRadians(mRotation.y),
			XMConvertToRadians(mRotation.z)
		);

		Quaternion q = Quaternion::CreateFromYawPitchRoll(rotationRad);
		mBasis = Matrix::CreateFromQuaternion(q);

		const float speed = 100.0f;

		mPosition += position.x * mBasis.Right() * speed * deltaTime;
		mPosition += position.y * mBasis.Up() * speed * deltaTime;
		mPosition += position.z * GetForwardDirection() * speed * deltaTime;

		Matrix world = mBasis * Matrix::CreateTranslation(mPosition);
		mViewMatrix = world.Invert();
	}

	if (inputManager.IsLeftButtonDown())
	{
		TryRemoveBlock();
	}
}

void Camera::TryRemoveBlock() const
{
	MapManager& mapManager = MapManager::GetInstance();
	
	Vector3 forward = GetForwardDirection();
	const float step = 0.1f;
	float o = 0.5f;
	while (o < RANGE)
	{
		Vector3 checkPos = mPosition + forward * o;
		o += step;

		if (mapManager.IsBlockAt(checkPos))
		{
			mapManager.RemoveBlockAt(checkPos);
			Logger::LogLine("블록 제거 at (%f, %f, %f)", checkPos.x, checkPos.y, checkPos.z);
			break;
		}
	}
}

void Camera::CreatePjoectionMatrix()
{
	constexpr float fovRadian = DirectX::XMConvertToRadians(FOV_DEGREES);
	const float aspectRatio = ScreenManager::GetInstance().GetClientAreaAspectRatio();
	const float nearZ = 0.1f;
	const float farZ = 500.0f;

	mProjMatrix = DirectX::XMMatrixPerspectiveFovLH(fovRadian, aspectRatio, nearZ, farZ);
}

