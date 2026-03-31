//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

#ifdef ENABLE_AUTOMATION_TEST
	#include "TestRunner.h"
#endif

#include <iostream>
#include <cassert>
#include <string>

#include "GLPlatform.h"
#include "ScreenManager.h"
#include "ImguiLayer.h"
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
#include "MapManager.h"
#include "GPUResourceService.h"
#include "TextureManager.h"
#include "JobScheduler.h"
#include "Renderer.h"

void GLFWErrorCallback(int errorCode, const char* description)
{
	// Callback으로 자동 발생
    std::cerr << "GLFW error (" << errorCode << "): " << description << '\n';
}


int main()
{
	glfwSetErrorCallback(GLFWErrorCallback);

	if (glfwInit() == false)
	{
		std::cerr << "glfwInit failed\n";
		return -1;
	}

	if (ScreenManager::GetInstance().CreateGlfwWindow("VoxelWorld") == false) 
	{
		std::cerr << "CreateGlfwWindow failed\n";
		glfwTerminate();
		return -1;
	}

	{
		GLFWwindow* window = ScreenManager::GetInstance().GetWindows();
		// OpenGL 컨텍스트 현재 스레드에 바인딩
		glfwMakeContextCurrent(window);
		// 드라이버에서 런타임으로 함수 포인터 갖고오기
		if (gladLoadGL((GLADloadfunc)glfwGetProcAddress) == false)
		{
			ScreenManager::GetInstance().DestoryGlfwWindow();
			glfwTerminate();
			return -1;
		}

		// GPU Resouce Service
		GPUResourceService gpuResourceService;
		{
			RuntimeConfig runtimeConfig;
			StreamingPolicy streamingPolicy(runtimeConfig);
			AdaptiveRenderDistanceController adaptiveRenderDistanceController(runtimeConfig);

			// Bundle 넣어줘야 함, Factory에서 생성
			ImguiLayerDesc layerDesc{ window, "#version 420 core" };
			ImguiLayer imGuiLayer(layerDesc);
			BlockMaterialTable blockMaterialTable = BlockLoader::Load(PathUtils::GetAssetPath("Atlas.json"));
			MeshBuilder meshBuilder(blockMaterialTable);

			// Texture Manager
			TextureManager textureManager(gpuResourceService);

			JobScheduler jobScheduler(meshBuilder, streamingPolicy);
			Renderer renderer(window, gpuResourceService, textureManager, jobScheduler, streamingPolicy);
			renderer.Create();
			renderer.SetupStaticPipelineState();
			// job
			// renderer

			InputManager inputManager(window);

			DebugUI debugUI;
			Profiler profiler;

			MapManager mapManager(streamingPolicy);
			Camera camera(Vector3(5.f, 20.f, 0.f), Vector3(0.f, 0.f, 0.f));

	#ifdef ENABLE_AUTOMATION_TEST
			TestRunner runner(Vector3(WorldConfig::WORLD_MIN_X, 30.f, WorldConfig::WORLD_MIN_Z));
			camera.SetPosition(runner.GetPostion());
	#endif

			Timer timer;
			timer.Reset();
			while (glfwWindowShouldClose(window) == false)
			{
				// OS 이벤트 처리
				glfwPollEvents();
				timer.Tick();

				float deltaTime = timer.GetDeltaTime();
				if (inputManager.Update(window) == false)
				{
					break;
				}
						// 얘네까지 Scene 안에 포함 될 
		#ifdef ENABLE_AUTOMATION_TEST
				// test 코드 작성
				CameraInput input = runner.Update(deltaTime);
				camera.Update(input, deltaTime, mapManager);
		#else
				camera.Update(inputManager, deltaTime, mapManager);
		#endif

				mapManager.Update(camera, renderer);

				imGuiLayer.BeginFrame();
				renderer.Update(camera, deltaTime, mapManager);

				profiler.UpdateFrameMetrics(timer);
				adaptiveRenderDistanceController.Update(profiler, deltaTime);

				DebugUIContext debugUIContext{ camera, runtimeConfig, mapManager, renderer };
				debugUI.Draw(profiler, debugUIContext);

				imGuiLayer.Render();

				// 둘이 합치기
				renderer.Present();
				// DX11의 Present 처럼 버퍼 교체
			}
			renderer.Release();
		}

#ifdef _DEBUG
		gpuResourceService.PrintDebugResourceStats();
#endif
	}
	
	ScreenManager::GetInstance().DestoryGlfwWindow();
	glfwTerminate();
	return 0;
}
