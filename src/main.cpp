//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

#ifdef ENABLE_AUTOMATION_TEST
	#include "TestRunner.h"
#endif

#include <iostream>
#include <cassert>
#include <string>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "ScreenManager.h"
#include "ImGuiLayer.h"
#include "RuntimeConfig.h"
#include "StreamingPolicy.h"
#include "AdaptiveRenderDistanceController.h"
#include "PathUtils.h"
#include "DebugUI.h"
#include "Timer.h"
#include "Logger.h"
#include "BlockLoader.h"
#include "BlockMaterialTable.h"
#include "Profiler.h"
#include "MeshBuilder.h"
#include "InputManager.h"
#include "Camera.h"

// #include "JobScheduler.h"
// #include "Renderer.h"
// #include "MapManager.h"
// #include "TextureManager.h"
// #include "GPUResourceService.h"

void GLFWErrorCallback(int errorCode, const char* description)
{
	// Callback으로 자동 발생
    std::cerr << "GLFW error (" << errorCode << "): " << description << '\n';
}


int main()
{
	glfwSetErrorCallback(GLFWErrorCallback);

	if (glfwInit() == false || ScreenManager::GetInstance().CreateGlfwWindow("VoxelWorld") == false)
	{
		glfwTerminate();
		return -1;
	}

	// 스왑체인 만들어주기

	GLFWwindow* window = ScreenManager::GetInstance().GetWindows();
	RuntimeConfig runtimeConfig;
	StreamingPolicy streamingPolicy(runtimeConfig);
	AdaptiveRenderDistanceController adaptiveRenderDistanceController(runtimeConfig);

	// OpenGL 컨텍스트 현재 스레드에 바인딩
	glfwMakeContextCurrent(window);

	// Bundle 넣어줘야 함
	ImGuiLayerDesc layerDesc{ window};
	ImGuiLayer imGuiLayer(layerDesc);
	BlockMaterialTable blockMaterialTable = BlockLoader::Load(PathUtils::GetAssetPath("Atlas.json"));
	MeshBuilder meshBuilder(blockMaterialTable);

	InputManager inputManager(window);

	DebugUI debugUI;
	Profiler profiler;

	Camera camera(Vector3(5.f, 20.f, 0.f), Vector3(0.f, 0.f, 0.f));

	Timer timer;
	timer.Reset();
	while (glfwWindowShouldClose(window) == false)
	{
		// OS 이벤트 처리
		glfwPollEvents();

		float deltaTime = timer.GetDeltaTime();
		if (inputManager.Update(window) == false)
		{
			break;
		}


		// DX11의 Present 처럼 버퍼 교체
		glfwSwapBuffers(window);
	}

	ScreenManager::GetInstance().DestoryGlfwWindow();
	glfwTerminate();
	return 0;
}

// int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
// {
// 	// Engine.
// 	WCHAR windowClass[] = L"Voxel World";
// 	WCHAR title[] = L"Voxel World";

// 	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, windowClass };
// 	RegisterClassW(&wndclass);

// 	ScreenManager::GetInstance().CreateHWND(windowClass, title, hInstance);
// 	HWND hWnd = ScreenManager::GetInstance().GetHWND();

// 	StreamingPolicy streamingPolicy(runtimeConfig);
// 	AdaptiveRenderDistanceController adaptiveRenderDistanceController(runtimeConfig);

// 	DeviceBundle deviceBundle = DeviceFactory::CreateDeviceAndSwapChain(hWnd);
// 	ImGuiLayerDesc layerDesc{ hWnd, deviceBundle.Device.Get(), deviceBundle.DeviceContext.Get() };
// 	ImGuiLayer imGuiLayer(layerDesc);

// 	DebugUI debugUI;
// 	Profiler profiler;

// 	GPUResourceService gpuResourceService(deviceBundle.Device, deviceBundle.DeviceContext);
// 	TextureManager textureManager(gpuResourceService);

// 	BlockMaterialTable blockMaterialTable = BlockLoader::Load(PathUtils::GetAssetPath("Atlas.json"));
// 	MeshBuilder meshBuilder(blockMaterialTable);
// 	JobScheduler jobScheduler(meshBuilder, streamingPolicy);
// 	Renderer renderer(deviceBundle, gpuResourceService, textureManager, jobScheduler, streamingPolicy);
// 	renderer.Create();
// 	renderer.SetupStaticPipelineState();

// 	MapManager mapManager(streamingPolicy);
// 	InputManager inputManager;

// 	Camera camera(Vector3(5, 20, 0), Vector3());

// #ifdef ENABLE_AUTOMATION_TEST
// 	TestRunner runner(Vector3(WorldConfig::WORLD_MIN_X, 30.f, WorldConfig::WORLD_MIN_Z));
// 	camera.SetPosition(runner.GetPostion());
// #endif

// 	Timer timer;
// 	timer.Reset();
	
// 	while (true)
// 	{
// 		timer.Tick();

// 		MSG msg;
// 		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
// 		{
// 			TranslateMessage(&msg);
// 			DispatchMessage(&msg);

// 			if (msg.message == WM_QUIT)
// 			{
// 				break;
// 			}
// 		}
// 		float deltaTime = timer.GetDeltaTime();

// 		if (inputManager.Update() == false)
// 		{
// 			break;
// 		}

// 		// 얘네까지 Scene 안에 포함 될 
// #ifdef ENABLE_AUTOMATION_TEST
// 		// test 코드 작성
// 		CameraInput input = runner.Update(deltaTime);
// 		camera.Update(input, deltaTime, mapManager);
// #else
		
// 		camera.Update(inputManager, deltaTime, mapManager);
// #endif
// 		mapManager.Update(camera, renderer);

// 		imGuiLayer.BeginFrame();
// 		renderer.Update(camera, deltaTime, mapManager);

// 		profiler.UpdateFrameMetrics(timer);
// 		adaptiveRenderDistanceController.Update(profiler, deltaTime);

// 		DebugUIContext debugUIContext{ camera, runtimeConfig, mapManager, renderer };
// 		debugUI.Draw(profiler, debugUIContext);

// 		imGuiLayer.Render();
// 		renderer.Present();
// 	}

// 	renderer.Release();

// 	// 여기서 프레임을 찍을까?


// 	return 0;
// }
