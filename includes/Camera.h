#pragma once
#include <SimpleMath.h>
#include <cstdint>

using namespace DirectX::SimpleMath;

class MapManager;
class InputManager;

struct CameraInput
{
	Vector3 KeyboardMovement;
	Vector3 MouseMovement;
	bool bIsLeftButtonDown;
	bool bShouldChangedSpeed;
};

class Camera
{
public:
	Camera(const Vector3 position, const Vector3 rotation);
	~Camera() = default;

	void Update(const InputManager& inputManager, const float deltaTime, MapManager& mapManager);
	void Update(const CameraInput& input, const float deltaTime, MapManager& mapManager);

	Vector3 GetPosition() const { return mPosition; }
	void SetPosition(const Vector3 position) { mPosition = position; }
	void SetRotation(const Vector3 rotation) { mRotation = rotation; }
	Matrix GetSkyboxViewMatrix() const { return mBasis.Invert(); }
	Matrix GetViewMatrix() const { return mViewMatrix; }
	Matrix GetProjectionMatrix() const { return mProjMatrix; }
	Matrix GetViewProjectionMatrix() const { return mViewProjMatrix; }
	Vector3 GetForwardDirection() const { return mBasis.Backward(); } // right-handed 를 기준으로 하는 함수
	bool HasTransformChanged() const { return mbTransformDirty; }
	bool IsChangedRenderDistance() const { return false; } // 나중에 구현

private:
	static constexpr uint32_t MAX_LEVEL = 3u;
	static constexpr float SPEEDS[MAX_LEVEL]{ 1.f, 20.f, 100.f };

	Vector3 mPosition;
	Vector3 mRotation;

	Matrix mBasis;
	Matrix mViewMatrix;
	Matrix mProjMatrix;
	Matrix mViewProjMatrix;

	bool mbTransformDirty;
	uint32_t mSpeedLevel;
	//bool mbIsDirty;


	void TryRemoveBlock(MapManager& mapManager) const;
	void CreatePjoectionMatrix();
	void CreateViewMatrix(const Vector3 position, const Vector3 mouseMovement, const float deltaTime);
	float GetCurrentSpeed() const;
};




