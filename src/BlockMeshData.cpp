#include "BlockMeshData.h"

void BlockMeshData::AddFace(std::vector<BlockVertex>& vertices, std::vector<uint32_t>& indices, Direction direction, IVector3 position)
{
	assert(static_cast<uint32_t>(direction) < static_cast<uint32_t>(Direction::Size));

	uint32_t dir = static_cast<uint32_t>(direction);
	uint32_t offset = vertices.size();

	for (uint32_t i = 0; i < POINT_COUNT; ++i)
	{
		BlockVertex p
		{
			.position = Vector3(position.x + verticesScaledBy2[dir][i].x, position.y + verticesScaledBy2[dir][i].y, position.z + verticesScaledBy2[dir][i].z),
			.normal = Vector3(normals[dir].x, normals[dir].y, normals[dir].z),
			.uv = Vector2(uvs[i].x, uvs[i].y)
		};
		vertices.emplace_back(p);
	}

	// index 6개 집어 넣기
	for (uint32_t j = 0; j < FACE_INDICES_PATTERN_SIZE; ++j)
	{
		indices.push_back(offset + faceIndicesPattern[j]);
	}
}