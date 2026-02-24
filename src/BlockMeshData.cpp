#include "BlockMeshData.h"

void BlockMeshData::AddFace(std::vector<BlockVertex>& vertices, std::vector<uint32_t>& indices, Direction direction, IVector3 position)
{
	assert(static_cast<uint32_t>(direction) < static_cast<uint32_t>(Direction::Size));

	uint32_t dir = static_cast<uint32_t>(direction);
	uint32_t offset = static_cast<uint32_t>(vertices.size());

	for (uint32_t i = 0; i < POINT_COUNT; ++i)
	{
		BlockVertex p
		{
			.position = Vector3(
				static_cast<float>(position.x) + static_cast<float>(verticesScaledBy2[dir][i].x) * 0.5f, 
				static_cast<float>(position.y) + static_cast<float>(verticesScaledBy2[dir][i].y) * 0.5f,
				static_cast<float>(position.z) + static_cast<float>(verticesScaledBy2[dir][i].z) * 0.5f),
			.normal = Vector3(static_cast<float>(normals[dir].x), static_cast<float>(normals[dir].y), static_cast<float>(normals[dir].z)),
			.uv = Vector2(static_cast<float>(uvs[i].x), static_cast<float>(uvs[i].y))
		};
		vertices.emplace_back(p);
	}

	for (uint32_t j = 0; j < FACE_INDICES_PATTERN_SIZE; ++j)
	{
		indices.push_back(offset + faceIndicesPattern[j]);
	}
}