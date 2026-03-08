#include "MeshBuilder.h"
#include "BlockMeshData.h"
#include "WorldConfig.h"

MeshBuilder::MeshBuilder()
{
	mMeshData.Vertices.reserve(MAX_VERTEX_COUNT);
	mMeshData.Indices.reserve(MAX_INDEX_COUNT);
}

const MeshData& MeshBuilder::Build(const Chunk& chunk)
{
	assert(chunk.IsDirty());

	mMeshData.Vertices.clear();
	mMeshData.Indices.clear();
	// chunk의 블록이 air인지

	constexpr uint32_t size = static_cast<uint32_t>(Direction::Size);
	constexpr IVector3 dxdydz[size] =
	{
		{ 0, 0, 1 }, // front
		{ 0, 0, -1 }, // back
		{ 1, 0, 0 }, // right
		{ -1, 0, 0 }, // left
		{ 0, 1, 0 }, // top
		{ 0, -1, 0 }, // bottom
	};

	static_assert(dxdydz[static_cast<uint32_t>(Direction::Front)] == IVector3(0, 0, 1), "dxdydz Order Error!");
	static_assert(dxdydz[static_cast<uint32_t>(Direction::Back)] == IVector3(0, 0, -1), "dxdydz Order Error!");
	static_assert(dxdydz[static_cast<uint32_t>(Direction::Right)] == IVector3(1, 0, 0), "dxdydz Order Error!");
	static_assert(dxdydz[static_cast<uint32_t>(Direction::Left)] == IVector3(-1, 0, 0), "dxdydz Order Error!");
	static_assert(dxdydz[static_cast<uint32_t>(Direction::Top)] == IVector3(0, 1, 0), "dxdydz Order Error!");
	static_assert(dxdydz[static_cast<uint32_t>(Direction::Bottom)] == IVector3(0, -1, 0), "dxdydz Order Error!");

	const int32_t chunkSize = WorldConfig::CHUNK_SIZE;
	const IVector3 chunkWorldPosition = chunk.GetChunkPosition();

	for (int32_t z = 0; z < chunkSize; ++z)
	{
		for (int32_t x = 0; x < chunkSize; ++x)
		{
			for (int32_t y = 0; y < chunkSize; ++y)
			{
				if (chunk.IsAir(x, y, z))
				{
					continue;
				}

				BlockType type = chunk.GetBlockType(x, y, z);

				for (uint32_t i = 0; i < size; ++i)
				{
					if (chunk.IsAir(x + dxdydz[i].x, y + dxdydz[i].y, z + dxdydz[i].z) == false)
					{
						continue;
					}
					BlockMeshData::AddFace(mMeshData.Vertices, mMeshData.Indices, static_cast<Direction>(i), IVector3(x, y, z), chunkWorldPosition, type);
				}
			}
		}
	}
	return mMeshData;
}
