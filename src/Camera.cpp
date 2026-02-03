#include <algorithm>
#include <string>
#include <DirectXMath.h>
#include "Camera.h"
#include "ScreenManager.h"
#include "InputManager.h"
#include "Logger.h"

Camera::Camera()
{
	CreatePjoectionMatrix();
}

Matrix Camera::GetViewMatrix() const
{
	return mViewMatrix;
}

Matrix Camera::GetProjectionMatrix() const
{
	return mProjMatrix;
}

void Camera::Update(const InputManager& inputManager)
{
	Vector3 position = inputManager.GetKeyboardMovement();
	Vector3 mouseMovement = inputManager.GetMouseMovement();

	Vector3 prevPosition = mPosition;
	Vector3 prevRotation = mRotation;

	const float sensitivity = 0.15f;
	mRotation.y += mouseMovement.x * sensitivity; // yaw 
	mRotation.x = std::clamp(mRotation.x + mouseMovement.y * sensitivity, -89.9f, 89.9f); // pitch

	Vector3 rotationRad = Vector3(
		XMConvertToRadians(mRotation.x),
		XMConvertToRadians(mRotation.y),
		XMConvertToRadians(mRotation.z)
	);

	Quaternion q = Quaternion::CreateFromYawPitchRoll(rotationRad);
	mBasis = Matrix::CreateFromQuaternion(q);

	const float speed = 0.1f;

	mPosition += position.x * mBasis.Right() * speed;
	mPosition += position.y * mBasis.Up() * speed;
	mPosition += position.z * mBasis.Backward() * speed;

	Matrix world = mBasis * Matrix::CreateTranslation(mPosition);
	mViewMatrix = world.Invert();
}

void Camera::CreatePjoectionMatrix()
{
	constexpr float fovDegree = 80.f;
	constexpr float fovRadian = DirectX::XMConvertToRadians(fovDegree);
	const float aspectRatio = ScreenManager::GetInstance().GetClientAreaAspectRatio();
	const float nearZ = 0.1f;
	const float farZ = 1000.0f;

	mProjMatrix = DirectX::XMMatrixPerspectiveFovLH(fovRadian, aspectRatio, nearZ, farZ);
}