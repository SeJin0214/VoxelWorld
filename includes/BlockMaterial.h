#pragma once
#include "Types.h"
#include "BlockTextureType.h"

struct AtlasXY
{
	uint32_t StartX;
	uint32_t StartY;
};

class BlockMaterial
{
public:
	BlockMaterial(const BlockTextureType top, const BlockTextureType bottom, const BlockTextureType side)
		: mTop(top)
		, mBottom(bottom)
		, mSide(side)
	{

	}

	BlockTextureType GetTop() const { return mTop; }
	BlockTextureType GetSide() const { return mSide; }
	BlockTextureType GetBottom() const { return mBottom; }

	static AtlasXY GetAtlasXY(const BlockTextureType type)
	{
		return sTextures[static_cast<uint32_t>(type)];
	}

private:
	BlockTextureType mTop;
	BlockTextureType mBottom;
	BlockTextureType mSide;

	static constexpr AtlasXY sTextures[static_cast<uint32_t>(BlockTextureType::Size)] =
	{
		{ 48, 48 },
		{ 0, 0 },
		{ 16, 0 },
		{ 32, 0 },
		{ 48, 0 },
		{ 0, 16 },
		{ 16, 16 },
		{ 0, 32 },
		{ 0, 48 }
	};
};