#include <windows.h>
#include <cassert>
#include <string>
#include "Renderer.h"
#include "ScreenManager.h"
#include "Logger.h"
#include "InputManager.h"
#include "Timer.h"
#include "MapManager.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
		DirectX::Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WCHAR windowClass[] = L"Voxel World";
	WCHAR title[] = L"Voxel World";

	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, windowClass };
	RegisterClassW(&wndclass);

	ScreenManager::GetInstance().CreateHWND(windowClass, title, hInstance);
	HWND hWnd = ScreenManager::GetInstance().GetHWND();

	Renderer renderer;
	renderer.Create(hWnd);
	renderer.PreparePipeline();

	BlockMeshData meshData;

	// VertexBuffer 생성하기
	ID3D11Buffer* vertexBuffer = renderer.CreateBlockMeshVertexBuffer(&meshData);
	ID3D11Buffer* indexBuffer = renderer.CreateIndexBuffer(&meshData);

	InputManager inputManager;
	Camera camera(Vector3(5, 0, -50), Vector3());


	Timer timer;
	timer.Reset();
	bool bIsExited = false;
	while (bIsExited == false)
	{
		timer.Tick();
		std::wstring title = L"GameEngine | FPS: " + std::to_wstring(static_cast<int>(timer.GetFPS()));
		SetWindowText(hWnd, title.c_str());

		MSG msg;
		// 처리할 메시지가 더 이상 없을때 까지 수행
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// 키 입력 메시지를 번역
			TranslateMessage(&msg);

			// 메시지를 적절한 윈도우 프로시저에 전달, 메시지가 위에서 등록한 WndProc 으로 전달됨
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				bIsExited = true;
				break;
			}
		}

		renderer.Prepare();



		inputManager.Update();

		float deltaTime = timer.GetDeltaTime();
		camera.Update(inputManager, deltaTime);

		// 나중에 경계를 제대로 잡아야 할 거 같다.
		
		MapManager& mapManager = MapManager::GetInstance();
		mapManager.Update(camera);

		// loadChunkMesh 


		const std::vector<const Chunk*> visibleChunks = mapManager.GetVisibleChunks();
		std::vector<Vector3> blockPositions;
		blockPositions.reserve(static_cast<size_t>(Chunk::GetChunkSize()) * Chunk::GetChunkSize() * Chunk::GetChunkSize());
		for (const Chunk* chunk : visibleChunks)
		{
			IVector3 chunkPosition = chunk->GetChunkPosition();
			renderer.UpdateConstantBuffer(camera, Vector3(static_cast<float>(chunkPosition.x), static_cast<float>(chunkPosition.y), static_cast<float>(chunkPosition.z)));

			chunk->GetBlockPositions(blockPositions);
			if (blockPositions.size() == 0)
			{
				continue;
			}
			ID3D11Buffer* instanceBuffer = renderer.CreateInstanceBuffer(blockPositions);

			UINT indexCount = sizeof(meshData.indices) / sizeof(UINT);
			// instance buffer 바인딩
			renderer.Render(vertexBuffer, indexBuffer, indexCount, instanceBuffer, static_cast<UINT>(blockPositions.size()));

			blockPositions.clear();
			instanceBuffer->Release();
		}
		

		//int count = 0;
		//for (int i = 0; i < mapManager.GetRowCount(); ++i)
		//{
		//	for (int j = 0; j < mapManager.GetColumnCount(); ++j)
		//	{
		//		for (int k = 0; k < mapManager.GetHightCount(); ++k)
		//		{
		//			if (mapManager.IsBlockAt(i, k, j) == false)
		//			{
		//				continue;
		//			}

		//			Vector3 cubePosition = Vector3(
		//				static_cast<float>(i),
		//				static_cast<float>(k),
		//				static_cast<float>(j)
		//			);

		//			renderer.UpdateConstantBuffer(camera, cubePosition);
		//			UINT indexCount = sizeof(meshData.indices) / sizeof(UINT);
		//			renderer.Render(vertexBuffer, indexBuffer, indexCount);
		//		}
		//	}
		//}

		renderer.Present();
		//renderer.UpdateConstantBuffer(camera, Vector3(0.f, 0.f, 0.f));
		//UINT indexCount = sizeof(meshData.indices) / sizeof(UINT);
		//renderer.Render(vertexBuffer, indexBuffer, indexCount);
	}

	renderer.ReleaseBuffer(indexBuffer);
	renderer.ReleaseBuffer(vertexBuffer);
	renderer.Release();

	return 0;
}