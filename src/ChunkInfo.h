#pragma once
#include "IVector3.h"

struct ChunkInfo
{
	IVector3 position;
	int32_t index;
	ChunkInfo(const IVector3 pos, const int32_t idx)
		: position(pos)
		, index(idx)
	{
	}
};