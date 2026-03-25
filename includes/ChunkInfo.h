#pragma once
#include "IVector3.h"

struct ChunkInfo
{
	IVector3 Position;
	int32_t Index;
	ChunkInfo(const IVector3 pos, const int32_t idx)
		: Position(pos)
		, Index(idx)
	{
	}
};