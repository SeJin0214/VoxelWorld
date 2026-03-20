#include "RuntimeConfig.h"
#include "WorldConfig.h"
#include "Logger.h"

RuntimeConfig::RuntimeConfig()
	: mRenderDistance(WorldConfig::DEFAULT_RENDER_DISTANCE)
{
}

void RuntimeConfig::SetRenderDistance(const int32_t renderDistance, const char* callerName)
{
	mRenderDistance = renderDistance; 
}