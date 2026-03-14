#pragma once
#include "Types.h"

class RuntimeConfig
{
public:
	RuntimeConfig();
	~RuntimeConfig() = default;
	RuntimeConfig(const RuntimeConfig& other) = delete;
	RuntimeConfig& operator=(const RuntimeConfig& rhs) = delete;

	int32_t GetRenderDistance() const { return mRenderDistance; }
	void SetRenderDistance(const int32_t renderDistance, const char* callerName);

private:
	int32_t mRenderDistance;
};