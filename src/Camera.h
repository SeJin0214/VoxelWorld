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
	Matrix GetViewMatrix() const { return mViewMatrix; }
	Matrix GetProjectionMatrix() const { return mProjMatrix; }
	Vector3 GetForwardDirection() const { return mBasis.Backward(); } // right-handed 를 기준으로 하는 함수
	void TryRemoveBlock();


private:
	static constexpr int RANGE = 5;

	Vector3 mPosition = Vector3(0, 0, -5.f);
	Vector3 mRotation;

	Matrix mBasis;
	Matrix mViewMatrix;
	Matrix mProjMatrix;

	void CreatePjoectionMatrix();
};