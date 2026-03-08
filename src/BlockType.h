#pragma once

enum class BlockType : uint8_t
{
	Air,

	// Surface
	Sand,
	Grass,
	SnowGrass,

	// Subsurface
	SandStone,
	Dirt,

	// Deep
	Stone,
};