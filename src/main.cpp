//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>


// 콘솔 출력 
#pragma comment(linker, "/subsystem:console")
#pragma comment(linker, "/entry:WinMainCRTStartup")
#include <iostream>

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

	//ID3D11Buffer* instanceBuffer = renderer.CreateInstanceBuffer(sizeof(Vector3) * Chunk::GetTotalChunkCount());

	InputManager inputManager;
	Camera camera(Vector3(5, 0, -50), Vector3());

	Timer timer;
	timer.Reset();
	bool bIsExited = false;
	while (bIsExited == false)
	{
		timer.Tick();
		std::wstring title = L"VoxelEngine | FPS: " + std::to_wstring(static_cast<int>(timer.GetFPS()));

		std::cout << " FPS:" << timer.GetFPS() << std::endl;

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

		if (inputManager.Update() == false)
		{
			break;
		}

		float deltaTime = timer.GetDeltaTime();
		camera.Update(inputManager, deltaTime);

		// 나중에 경계를 제대로 잡아야 할 거 같다.
		
		MapManager& mapManager = MapManager::GetInstance();
		mapManager.Update(camera, renderer);

		renderer.Update(camera);

	}

	//renderer.ReleaseBuffer(instanceBuffer);
	renderer.Release();


	//_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	//_CrtDumpMemoryLeaks();

	return 0;
}