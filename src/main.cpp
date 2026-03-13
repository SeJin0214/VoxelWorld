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

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

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

	// Engine.cpp

	WCHAR windowClass[] = L"Voxel World";
	WCHAR title[] = L"Voxel World";

	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, windowClass };
	RegisterClassW(&wndclass);

	ScreenManager::GetInstance().CreateHWND(windowClass, title, hInstance);
	HWND hWnd = ScreenManager::GetInstance().GetHWND();

	RuntimeConfig runtimeConfig;
	StreamingPolicy streamingPolicy(runtimeConfig);
	AdaptiveRenderDistanceController adaptiveRenderDistanceController(runtimeConfig);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	DeviceFactory::DeviceBundle deviceBundle = DeviceFactory::CreateDeviceAndSwapChain(hWnd);

	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(deviceBundle.Device.Get(), deviceBundle.DeviceContext.Get());

	GPUResourceService gpuResourceService(deviceBundle.Device, deviceBundle.DeviceContext);
	TextureManager textureManager(gpuResourceService);

	BlockMaterialTable blockMaterialTable = BlockLoader::Load(PathUtils::GetAssetPath("Atlas.json"));
	MeshBuilder meshBuilder(blockMaterialTable);
	// MeshBuilder에게 BlockMaterialTable 주입하고, MeshBuilder를 Renderer에게 주입하기
	Renderer renderer(deviceBundle, gpuResourceService, textureManager, meshBuilder, streamingPolicy);
	renderer.Create();
	renderer.SetupStaticPipelineState();

	MapManager mapManager(streamingPolicy);
	InputManager inputManager;

	Camera camera(Vector3(5, 0, -50), Vector3());

	Timer timer;
	timer.Reset();

	static uint32_t frameNumber = 0;
	timer.InitFPSStats();
	while (true)
	{
		++frameNumber;
		timer.Tick();
		timer.UpdateFPSStats();

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
				break;
			}
		}

		if (inputManager.Update() == false)
		{
			break;
		}

		float deltaTime = timer.GetDeltaTime();
		camera.Update(inputManager, deltaTime, mapManager);

		//printf("%f\n", timer.GetMonotonicSeconds());
		adaptiveRenderDistanceController.Update(deltaTime, timer.GetFPS(), timer.GetMonotonicSeconds());
		
		mapManager.Update(camera, renderer);


		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.35f);

		ImGui::Begin("Performance", nullptr,
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text("FPS: %d", timer.GetFPS());
		ImGui::Text("Frame Time: %.2f ms", timer.GetDeltaTime() * 1000.0f);

		ImGui::End();

		renderer.Update(camera, deltaTime, mapManager);

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		renderer.Present();

		timer.RenderFPSLog();
	}

	renderer.Release();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	//_CrtDumpMemoryLeaks();

	return 0;
}













