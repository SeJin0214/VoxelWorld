#include "BlockMeshData.h"

void BlockMeshData::AddFace(std::vector<BlockVertex>& vertices, std::vector<uint32_t>& indices, 
	const Direction direction, const IVector3 position, const IVector3 chunkWorldPosition, const Vector2 uv)
{
	assert(static_cast<uint32_t>(direction) < static_cast<uint32_t>(Direction::Size));

	uint32_t dir = static_cast<uint32_t>(direction);
	uint32_t offset = static_cast<uint32_t>(vertices.size());

	Vector2 uvs[4] =
	{
		uv,
		Vector2(uv.x + 0.25f, uv.y),
		Vector2(uv.x + 0.25f, uv.y + 0.25f),
		Vector2(uv.x, uv.y + 0.25f),
	};

	for (uint32_t i = 0; i < POINT_COUNT; ++i)
	{
		Vector2 currentUV = Vector2(static_cast<float>(uvs[i].x), static_cast<float>(uvs[i].y));
		// BlockType에 따른 UV 좌표 계산하기, 일단은 type 무시하고 uv 고정
		BlockVertex p
		{
			// texture ID도 넣어야 한다.
			.position = Vector3(
				static_cast<float>(chunkWorldPosition.x + position.x) + static_cast<float>(verticesScaledBy2[dir][i].x) * 0.5f,
				static_cast<float>(chunkWorldPosition.y + position.y) + static_cast<float>(verticesScaledBy2[dir][i].y) * 0.5f,
				static_cast<float>(chunkWorldPosition.z + position.z) + static_cast<float>(verticesScaledBy2[dir][i].z) * 0.5f),
			.normal = Vector3(static_cast<float>(normals[dir].x), static_cast<float>(normals[dir].y), static_cast<float>(normals[dir].z)),
			.uv = currentUV,
		};
		vertices.emplace_back(p);
	}

	for (uint32_t j = 0; j < FACE_INDICES_PATTERN_SIZE; ++j)
	{
		indices.push_back(offset + faceIndicesPattern[j]);
	}
}