#include <windows.h>
#include <cassert>
#include "Renderer.h"
#include "ScreenManager.h"
#include "Logger.h"
#include "InputManager.h"


// 전역 혹은 클래스 멤버로 선언

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
		Logger::LogLine("WM_INPUT incoming!\n");
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

static const BlockVertex gCubeVerts[24] =
{
	// 정점 구조: { {pos}, {normal}, {color}, {uv} }

	// +Z (Front)
	{ {-0.5f,  0.5f,  0.5f}, { 0, 0, 1}, {1, 1, 1, 1}, {0, 0} }, // 0
	{ { 0.5f,  0.5f,  0.5f}, { 0, 0, 1}, {1, 1, 1, 1}, {1, 0} }, // 1
	{ { 0.5f, -0.5f,  0.5f}, { 0, 0, 1}, {1, 1, 1, 1}, {1, 1} }, // 2
	{ {-0.5f, -0.5f,  0.5f}, { 0, 0, 1}, {1, 1, 1, 1}, {0, 1} }, // 3

	// -Z (Back)
	{ { 0.5f,  0.5f, -0.5f}, { 0, 0,-1}, {1, 1, 1, 1}, {0, 0} }, // 4
	{ {-0.5f,  0.5f, -0.5f}, { 0, 0,-1}, {1, 1, 1, 1}, {1, 0} }, // 5
	{ {-0.5f, -0.5f, -0.5f}, { 0, 0,-1}, {1, 1, 1, 1}, {1, 1} }, // 6
	{ { 0.5f, -0.5f, -0.5f}, { 0, 0,-1}, {1, 1, 1, 1}, {0, 1} }, // 7

	// +X (Right)
	{ { 0.5f,  0.5f,  0.5f}, { 1, 0, 0}, {1, 1, 1, 1}, {0, 0} }, // 8
	{ { 0.5f,  0.5f, -0.5f}, { 1, 0, 0}, {1, 1, 1, 1}, {1, 0} }, // 9
	{ { 0.5f, -0.5f, -0.5f}, { 1, 0, 0}, {1, 1, 1, 1}, {1, 1} }, // 10
	{ { 0.5f, -0.5f,  0.5f}, { 1, 0, 0}, {1, 1, 1, 1}, {0, 1} }, // 11

	// -X (Left)
	{ {-0.5f,  0.5f, -0.5f}, {-1, 0, 0}, {1, 1, 1, 1}, {0, 0} }, // 12
	{ {-0.5f,  0.5f,  0.5f}, {-1, 0, 0}, {1, 1, 1, 1}, {1, 0} }, // 13
	{ {-0.5f, -0.5f,  0.5f}, {-1, 0, 0}, {1, 1, 1, 1}, {1, 1} }, // 14
	{ {-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {1, 1, 1, 1}, {0, 1} }, // 15

	// +Y (Top)
	{ {-0.5f,  0.5f, -0.5f}, { 0, 1, 0}, {1, 1, 1, 1}, {0, 0} }, // 16
	{ { 0.5f,  0.5f, -0.5f}, { 0, 1, 0}, {1, 1, 1, 1}, {1, 0} }, // 17
	{ { 0.5f,  0.5f,  0.5f}, { 0, 1, 0}, {1, 1, 1, 1}, {1, 1} }, // 18
	{ {-0.5f,  0.5f,  0.5f}, { 0, 1, 0}, {1, 1, 1, 1}, {0, 1} }, // 19

	// -Y (Bottom)
	{ {-0.5f, -0.5f,  0.5f}, { 0,-1, 0}, {1, 1, 1, 1}, {0, 0} }, // 20
	{ { 0.5f, -0.5f,  0.5f}, { 0,-1, 0}, {1, 1, 1, 1}, {1, 0} }, // 21
	{ { 0.5f, -0.5f, -0.5f}, { 0,-1, 0}, {1, 1, 1, 1}, {1, 1} }, // 22
	{ {-0.5f, -0.5f, -0.5f}, { 0,-1, 0}, {1, 1, 1, 1}, {0, 1} }, // 23
};

static const UINT gCubeIndices[36] =
{
	0,2,1,  0,3,2,         // Front
	4,6,5,  4,7,6,         // Back
	8,10,9, 8,11,10,       // Right
	12,14,13, 12,15,14,    // Left
	16,18,17, 16,19,18,    // Top
	20,22,21, 20,23,22     // Bottom
};

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
	memcpy(meshData.vertices, gCubeVerts, sizeof(meshData.vertices));
	assert(sizeof(meshData.indices) == sizeof(UINT) * 36);
	memcpy(meshData.indices, gCubeIndices, sizeof(meshData.indices));


	// TODO : 프레임 레이트 조절
	const int targetFPS = 30;
	const double targetFrameTime = 1000.0 / targetFPS;

	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	LARGE_INTEGER startTime, endTime;
	double elapsedTime = 0.0;

	// VertexBuffer 생성하기
	ID3D11Buffer* vertexBuffer = renderer.CreateVertexBuffer(&meshData);
	ID3D11Buffer* indexBuffer = renderer.CreateIndexBuffer(&meshData);

	InputManager inputManager;
	Camera camera;

	bool bIsExited = false;

	while (bIsExited == false)
	{
		QueryPerformanceCounter(&startTime);
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

		camera.Update(inputManager);

		// 나중에 경계를 제대로 잡아야 할 거 같다. 
		renderer.UpdateConstantBuffer(camera);

		UINT indexCount = sizeof(meshData.indices) / sizeof(UINT);
		renderer.Render(vertexBuffer, indexBuffer, indexCount);

		do
		{
			Sleep(0);

			QueryPerformanceCounter(&endTime);

			elapsedTime = (endTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;

		} while (elapsedTime < targetFrameTime);
	}

	renderer.ReleaseBuffer(indexBuffer);
	renderer.ReleaseBuffer(vertexBuffer);
	renderer.Release();

	return 0;
}