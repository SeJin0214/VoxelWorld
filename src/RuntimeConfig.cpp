#include "RuntimeConfig.h"
#include "WorldConfig.h"
#include "Logger.h"

RuntimeConfig::RuntimeConfig()
	: mRenderDistance(WorldConfig::DEFAULT_RENDER_DISTANCE)
{
}

void RuntimeConfig::SetRenderDistance(const int32_t renderDistance, const char* callerName)
{
	Logger::LogLine("%s: ·»´õ °Å¸® %d -> %d", callerName, mRenderDistance, renderDistance);
	mRenderDistance = renderDistance; 
}