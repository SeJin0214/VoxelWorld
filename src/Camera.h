#pragma once
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class InputManager;

class Camera
{

public:
	Camera();
	~Camera() = default;

	void Update(const InputManager& inputManager, const float deltaTime);
	Matrix GetViewMatrix() const;
	Matrix GetProjectionMatrix() const;

private:
	Vector3 mPosition = Vector3(0, 0, -5.f);
	Vector3 mRotation;

	Matrix mBasis;
	Matrix mViewMatrix;
	Matrix mProjMatrix;

	void CreatePjoectionMatrix();
};