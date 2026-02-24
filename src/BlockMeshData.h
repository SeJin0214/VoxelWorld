#pragma once

#include "BlockVertex.h"
#include "IVector3.h"
#include <cstdint>

enum class Direction
{
	Front,
	Back,
	Right,
	Left,
	Top,
	Bottom,
	Size
};

class BlockMeshData
{
public:
	static void AddFace(std::vector<BlockVertex>& vertices, std::vector<uint32_t>& indices, Direction direction, IVector3 position);

private:
	static constexpr uint32_t FACE_COUNT = static_cast<uint32_t>(Direction::Size);
	static constexpr uint32_t FACE_INDICES_PATTERN_SIZE = 6;
	static constexpr uint32_t faceIndicesPattern[FACE_INDICES_PATTERN_SIZE] = { 0, 2, 1, 0, 3, 2 };

	static constexpr IVector3 normals[FACE_COUNT] =
	{
		{ 0, 0, 1 }, // Front
		{ 0, 0,-1 }, // Back
		{ 1, 0, 0 }, // Right
		{-1, 0, 0 }, // Left
		{ 0, 1, 0 }, // Top
		{ 0,-1, 0 }, // Bottom
	};

	static_assert(normals[static_cast<uint32_t>(Direction::Front)] == IVector3(0, 0, 1), "normals Order Error!");
	static_assert(normals[static_cast<uint32_t>(Direction::Back)] == IVector3(0, 0, -1), "normals Order Error!");
	static_assert(normals[static_cast<uint32_t>(Direction::Right)] == IVector3(1, 0, 0), "normals Order Error!");
	static_assert(normals[static_cast<uint32_t>(Direction::Left)] == IVector3(-1, 0, 0), "normals Order Error!");
	static_assert(normals[static_cast<uint32_t>(Direction::Top)] == IVector3(0, 1, 0), "normals Order Error!");
	static_assert(normals[static_cast<uint32_t>(Direction::Bottom)] == IVector3(0, -1, 0), "normals Order Error!");

	static constexpr uint32_t UV_COUNT = 4;

    // Keep UV data in IVector3 for compile-time static_assert checks.
    // At runtime, only x(u) and y(v) are used.
	static constexpr IVector3 uvs[UV_COUNT] =
	{
		{ 0, 0, 0 },
		{ 1, 0, 0 },
		{ 1, 1, 0 }, 
		{ 0, 1, 0 },
	};

	static_assert(uvs[0] == IVector3(0, 0, 0), "uv[0] vertex mismatch");
	static_assert(uvs[1] == IVector3(1, 0, 0), "uv[1] vertex mismatch");
	static_assert(uvs[2] == IVector3(1, 1, 0), "uv[2] vertex mismatch");
	static_assert(uvs[3] == IVector3(0, 1, 0), "uv[3] vertex mismatch");

	static constexpr uint32_t POINT_COUNT = 4;

    // Store positions as integer coordinates scaled by 2 for static_assert validation.
    // Runtime float position = value * 0.5f.
	static constexpr IVector3 verticesScaledBy2[FACE_COUNT][POINT_COUNT] =
	{
		{ // +Z (Front)
			{-1,  1,  1}, // 0
			{ 1,  1,  1}, // 1
			{ 1, -1,  1}, // 2
			{-1, -1,  1}, // 3
		},

		{ // -Z (Back)
			{ 1,  1, -1}, // 4
			{-1,  1, -1}, // 5
			{-1, -1, -1}, // 6
			{ 1, -1, -1}, // 7
		},

		{ // +X (Right)
			{ 1,  1,  1}, // 8
			{ 1,  1, -1}, // 9
			{ 1, -1, -1}, // 10
			{ 1, -1,  1}, // 11
		},

		// -X (Left)
		{
			{-1,  1, -1}, // 12
			{-1,  1,  1}, // 13
			{-1, -1,  1}, // 14
			{-1, -1, -1}, // 15
		},

		// +Y (Top)
		{
			{-1,  1, -1}, // 16
			{ 1,  1, -1}, // 17
			{ 1,  1,  1}, // 18
			{-1,  1,  1}, // 19
		},

		// -Y (Bottom)
		{
			{-1, -1,  1}, // 20
			{ 1, -1,  1}, // 21
			{ 1, -1, -1}, // 22
			{-1, -1, -1}, // 23
		},
	};

	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Front)][0] == IVector3(-1, 1, 1), "Front[0] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Front)][1] == IVector3(1, 1, 1), "Front[1] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Front)][2] == IVector3(1, -1, 1), "Front[2] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Front)][3] == IVector3(-1, -1, 1), "Front[3] vertex mismatch");

	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Back)][0] == IVector3(1, 1, -1), "Back[0] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Back)][1] == IVector3(-1, 1, -1), "Back[1] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Back)][2] == IVector3(-1, -1, -1), "Back[2] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Back)][3] == IVector3(1, -1, -1), "Back[3] vertex mismatch");

	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Right)][0] == IVector3(1, 1, 1), "Right[0] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Right)][1] == IVector3(1, 1, -1), "Right[1] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Right)][2] == IVector3(1, -1, -1), "Right[2] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Right)][3] == IVector3(1, -1, 1), "Right[3] vertex mismatch");

	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Left)][0] == IVector3(-1, 1, -1), "Left[0] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Left)][1] == IVector3(-1, 1, 1), "Left[1] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Left)][2] == IVector3(-1, -1, 1), "Left[2] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Left)][3] == IVector3(-1, -1, -1), "Left[3] vertex mismatch");

	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Top)][0] == IVector3(-1, 1, -1), "Top[0] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Top)][1] == IVector3(1, 1, -1), "Top[1] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Top)][2] == IVector3(1, 1, 1), "Top[2] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Top)][3] == IVector3(-1, 1, 1), "Top[3] vertex mismatch");

	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Bottom)][0] == IVector3(-1, -1, 1), "Bottom[0] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Bottom)][1] == IVector3(1, -1, 1), "Bottom[1] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Bottom)][2] == IVector3(1, -1, -1), "Bottom[2] vertex mismatch");
	static_assert(verticesScaledBy2[static_cast<uint32_t>(Direction::Bottom)][3] == IVector3(-1, -1, -1), "Bottom[3] vertex mismatch");


	//static constexpr BlockVertex mVertices[24] =
	//{
	//	// { {pos}, {normal}, {uv} }

	//		// +Z (Front)
			//{ {-0.5f, 0.5f, 0.5f}, { 0, 0, 1 }, { 0, 0 } }, // 0
			//{ { 0.5f,  0.5f,  0.5f}, { 0, 0, 1}, {1, 0} }, // 1
			//{ { 0.5f, -0.5f,  0.5f}, { 0, 0, 1}, {1, 1} }, // 2
			//{ {-0.5f, -0.5f,  0.5f}, { 0, 0, 1}, {0, 1} }, // 3

			//// -Z (Back)
			//{ { 0.5f,  0.5f, -0.5f}, { 0, 0,-1}, {0, 0} }, // 4
			//{ {-0.5f,  0.5f, -0.5f}, { 0, 0,-1}, {1, 0} }, // 5
			//{ {-0.5f, -0.5f, -0.5f}, { 0, 0,-1}, {1, 1} }, // 6
			//{ { 0.5f, -0.5f, -0.5f}, { 0, 0,-1}, {0, 1} }, // 7

			//// +X (Right)
			//{ { 0.5f,  0.5f,  0.5f}, { 1, 0, 0}, {0, 0} }, // 8
			//{ { 0.5f,  0.5f, -0.5f}, { 1, 0, 0}, {1, 0} }, // 9
			//{ { 0.5f, -0.5f, -0.5f}, { 1, 0, 0}, {1, 1} }, // 10
			//{ { 0.5f, -0.5f,  0.5f}, { 1, 0, 0}, {0, 1} }, // 11

			//// -X (Left)
			//{ {-0.5f,  0.5f, -0.5f}, {-1, 0, 0}, {0, 0} }, // 12
			//{ {-0.5f,  0.5f,  0.5f}, {-1, 0, 0}, {1, 0} }, // 13
			//{ {-0.5f, -0.5f,  0.5f}, {-1, 0, 0}, {1, 1} }, // 14
			//{ {-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 1} }, // 15

			//// +Y (Top)
			//{ {-0.5f,  0.5f, -0.5f}, { 0, 1, 0}, {0, 0} }, // 16
			//{ { 0.5f,  0.5f, -0.5f}, { 0, 1, 0}, {1, 0} }, // 17
			//{ { 0.5f,  0.5f,  0.5f}, { 0, 1, 0}, {1, 1} }, // 18
			//{ {-0.5f,  0.5f,  0.5f}, { 0, 1, 0}, {0, 1} }, // 19

			//// -Y (Bottom)
			//{ {-0.5f, -0.5f,  0.5f}, { 0,-1, 0}, {0, 0} }, // 20
			//{ { 0.5f, -0.5f,  0.5f}, { 0,-1, 0}, {1, 0} }, // 21
			//{ { 0.5f, -0.5f, -0.5f}, { 0,-1, 0}, {1, 1} }, // 22
			//{ {-0.5f, -0.5f, -0.5f}, { 0,-1, 0}, {0, 1} }, // 23
	//};

	//static constexpr UINT mIndices[36] =
	//{
	//	0,2,1,  0,3,2,         // Front
	//	4,6,5,  4,7,6,         // Back
	//	8,10,9, 8,11,10,       // Right
	//	12,14,13, 12,15,14,    // Left
	//	16,18,17, 16,19,18,    // Top
	//	20,22,21, 20,23,22     // Bottom
	//};
};	
