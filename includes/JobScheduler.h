#pragma once
#include <queue>
#include "Types.h"
#include "IVector3.h"
#include "MeshData.h"

using std::queue;

class Renderer;
class MapManager;
class MeshBuilder;
class StreamingPolicy;

class JobScheduler
{
public:
	JobScheduler(MeshBuilder& meshBuilder, StreamingPolicy& streamingPolicy);
	~JobScheduler() = default;
	JobScheduler(const JobScheduler& other) = delete;
	JobScheduler& operator=(const JobScheduler& rhs) = delete;

	void RequestBuild(const ChunkKey key) { mRequestBuildJobs.push(key); }
	void ReleaseMeshData(MeshData* meshData);
	void ProcessBuild(Renderer& renderer, MapManager& mapManager, const IVector3 cameraChunkPos);

private:
	static constexpr uint32_t MESH_MAX_CREATE_COUNT_PER_FRAME = 4;

	MeshBuilder& mMeshBuilder;
	StreamingPolicy& mStreamingPolicy;
	queue<ChunkKey> mRequestBuildJobs;
	queue<ChunkKey> mPendingBuildJobs;

	
};