#pragma once
#include <SimpleMath.h>
#include <cstdint>

using namespace DirectX::SimpleMath;

class InputManager;

class Camera
{

public:
	Camera(const Vector3 position, const Vector3 rotation);
	~Camera() = default;

	void Update(const InputManager& inputManager, const float deltaTime);

	Vector3 GetPosition() const { return mPosition; }
	Matrix GetViewMatrix() const { return mViewMatrix; }
	Matrix GetProjectionMatrix() const { return mProjMatrix; }
	Vector3 GetForwardDirection() const { return mBasis.Backward(); } // right-handed 를 기준으로 하는 함수
	uint32_t GetRenderDistance() const { return mRenderDistance; }
	bool HasTransformChanged() const { return mbTransformDirty; }
	bool IsChangedRenderDistance() const { return false; } // 나중에 구현


private:
	static constexpr int32_t RANGE = 5;
	static constexpr float FOV_DEGREES = 80.f;

	Vector3 mPosition;
	Vector3 mRotation;

	Matrix mBasis;
	Matrix mViewMatrix;
	Matrix mProjMatrix;

	int32_t mRenderDistance;
	bool mbTransformDirty;
	//bool mbIsDirty;
	// IsChangedRenderDistance()
	// 나중에 설정 창에서 조절 가능하게

	void TryRemoveBlock() const;
	void CreatePjoectionMatrix();
};