#pragma once
#include <cstdint>
#include "Types.h"

class MapManager;
class InputManager;

struct CameraInput
{
	Vector3 KeyboardMovement;
	Vector2 MouseMovement;
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
	Matrix GetSkyboxViewMatrix() const { return glm::inverse(mBasis); }
	Matrix GetViewMatrix() const { return mViewMatrix; }
	Matrix GetProjectionMatrix() const { return mProjMatrix; }
	Matrix GetViewProjectionMatrix() const { return mViewProjMatrix; }
	Vector3 GetForwardDirection() const { return -Vector3(mBasis[2]); }
	Vector3 GetRightDirection() const { return Vector3(mBasis[0]); }
	Vector3 GetUpDirection() const { return Vector3(mBasis[1]); }

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





