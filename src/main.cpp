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
#include "BlockLoader.h"
#include "Renderer.h"
#include "ScreenManager.h"
#include "Logger.h"
#include "InputManager.h"
#include "Timer.h"
#include "MapManager.h"
#include "ScopedProfiler.h"
#include "TextureManager.h"
#include "BlockMaterialTable.h"
#include "PathUtils.h"
#include "RuntimeConfig.h"
#include "StreamingPolicy.h"
#include "AdaptiveRenderDistanceController.h"
#include "ImGuiLayer.h"
#include "DebugUI.h"
#include "Profiler.h"
#include "GPUResourceService.h"
#include "JobScheduler.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return true;
	}

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
	// Engine.
	WCHAR windowClass[] = L"Voxel World";
	WCHAR title[] = L"Voxel World";

	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, windowClass };
	RegisterClassW(&wndclass);

	ScreenManager::GetInstance().CreateHWND(windowClass, title, hInstance);
	HWND hWnd = ScreenManager::GetInstance().GetHWND();

	RuntimeConfig runtimeConfig;
	StreamingPolicy streamingPolicy(runtimeConfig);
	AdaptiveRenderDistanceController adaptiveRenderDistanceController(runtimeConfig);

	DeviceBundle deviceBundle = DeviceFactory::CreateDeviceAndSwapChain(hWnd);
	ImGuiLayerDesc layerDesc{ hWnd, deviceBundle.Device.Get(), deviceBundle.DeviceContext.Get() };
	ImGuiLayer imGuiLayer(layerDesc);

	DebugUI debugUI;
	Profiler profiler;

	GPUResourceService gpuResourceService(deviceBundle.Device, deviceBundle.DeviceContext);
	TextureManager textureManager(gpuResourceService);

	BlockMaterialTable blockMaterialTable = BlockLoader::Load(PathUtils::GetAssetPath("Atlas.json"));
	MeshBuilder meshBuilder(blockMaterialTable);
	JobScheduler jobScheduler(meshBuilder, streamingPolicy);
	Renderer renderer(deviceBundle, gpuResourceService, textureManager, jobScheduler, streamingPolicy);
	renderer.Create();
	renderer.SetupStaticPipelineState();

	MapManager mapManager(streamingPolicy);
	InputManager inputManager;

	Camera camera(Vector3(5, 0, -50), Vector3());

	Timer timer;
	timer.Reset();

	static uint32_t frameNumber = 0;
	while (true)
	{
		++frameNumber;
		timer.Tick();

		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				break;
			}
		}

		if (inputManager.Update() == false)
		{
			break;
		}

		float deltaTime = timer.GetDeltaTime();
		camera.Update(inputManager, deltaTime, mapManager);

		mapManager.Update(camera, renderer);

		imGuiLayer.BeginFrame();

		renderer.Update(camera, deltaTime, mapManager);

		profiler.UpdateFrameMetrics(timer);
		adaptiveRenderDistanceController.Update(profiler, deltaTime);

		DebugUIContext debugUIContext{ runtimeConfig, mapManager, renderer };
		debugUI.Draw(profiler, debugUIContext);

		imGuiLayer.Render();
		renderer.Present();
	}

	renderer.Release();

	return 0;
}
