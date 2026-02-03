#include <windows.h>
#include <cassert>
#include "Renderer.h"
#include "ScreenManager.h"
#include "Logger.h"
#include "InputManager.h"
#include "Timer.h"

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
	ID3D11Buffer* vertexBuffer = renderer.CreateVertexBuffer(&meshData);
	ID3D11Buffer* indexBuffer = renderer.CreateIndexBuffer(&meshData);

	InputManager inputManager;
	Camera camera;


	Timer timer;
	timer.Reset();
	bool bIsExited = false;
	while (bIsExited == false)
	{
		timer.Tick();

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

		inputManager.Update();

		float deltaTime = timer.GetDeltaTime();
		Logger::LogLine("Delta Time: %.4f sec", deltaTime);
		camera.Update(inputManager, deltaTime);

		// 나중에 경계를 제대로 잡아야 할 거 같다. 
		renderer.UpdateConstantBuffer(camera);

		UINT indexCount = sizeof(meshData.indices) / sizeof(UINT);
		renderer.Render(vertexBuffer, indexBuffer, indexCount);
	}

	renderer.ReleaseBuffer(indexBuffer);
	renderer.ReleaseBuffer(vertexBuffer);
	renderer.Release();

	return 0;
}