#include "JobScheduler.h"
#include "MapManager.h"
#include "Renderer.h"
#include "MeshBuilder.h"
#include "StreamingPolicy.h"

JobScheduler::JobScheduler(MeshBuilder& meshBuilder, StreamingPolicy& streamingPolicy)
	: mMeshBuilder(meshBuilder)
	, mStreamingPolicy(streamingPolicy)
{
}

void JobScheduler::ProcessBuild(Renderer& renderer, MapManager& mapManager, const IVector3 cameraChunkPos)
{
	uint32_t buildCount = 0;
	while (mRequestBuildJobs.empty() == false)
	{
		mPendingBuildJobs.push(mRequestBuildJobs.front());
		mRequestBuildJobs.pop();
	}

	uint32_t i = 0; 
	while (i < MESH_MAX_CREATE_COUNT_PER_FRAME && mPendingBuildJobs.empty() == false && mMeshBuilder.CanBuild())
	{
		++i;
		
		ChunkKey key = mPendingBuildJobs.front();
		mPendingBuildJobs.pop();
		if (mapManager.HasChunk(key) == false)
		{
			continue;
		}

		const Chunk& chunk = mapManager.GetChunk(key);
		if (mStreamingPolicy.ShouldKeep(chunk.GetChunkPosition(), cameraChunkPos) == false)
		{
			continue;
		}

		MeshData* mesh = mMeshBuilder.Build(chunk);
		mesh->Key = key;
		renderer.PushCompletedChunkMesh(mesh);

		++buildCount;
	}
	//printf("buildCount %u\n", buildCount);
}

void JobScheduler::ReleaseMeshData(MeshData* meshData)
{ 
	mMeshBuilder.DespawnMesh(meshData); 
}
