#pragma once

enum class BlockType : uint8_t
{
	Air,

	// Surface
	Sand,
	Grass,
	Snow,

	// Subsurface
	Dirt,

	// Deep
	Stone,
};