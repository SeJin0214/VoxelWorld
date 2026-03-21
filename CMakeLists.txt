cmake_minimum_required(VERSION 3.21)

set(PROJECT_NAME VoxelWorld)

project(${PROJECT_NAME} CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC_DIR src)
set(INC_DIR includes)
set(IMGUI_DIR third_party/imgui)
set(IMGUI_INC_DIR third_party/imgui)
set(XTK_DIR third_party/directxtk)

# PDB 
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>")

set(SRCS
	${SRC_DIR}/AdaptiveRenderDistanceController.cpp
	${SRC_DIR}/BiomePolicy.cpp
	${SRC_DIR}/BlockLoader.cpp
	${SRC_DIR}/BlockMeshData.cpp
	${SRC_DIR}/BufferPool.cpp
	${SRC_DIR}/Camera.cpp
	${SRC_DIR}/Chunk.cpp
	${SRC_DIR}/DeviceFactory.cpp
	${SRC_DIR}/GPUResourceService.cpp
	${SRC_DIR}/InputManager.cpp
	${SRC_DIR}/Logger.cpp
	${SRC_DIR}/main.cpp
	${SRC_DIR}/MapManager.cpp
	${SRC_DIR}/MeshBuilder.cpp
	${SRC_DIR}/PathUtils.cpp
	${SRC_DIR}/Renderer.cpp
	${SRC_DIR}/RuntimeConfig.cpp
	${SRC_DIR}/ScreenManager.cpp
	${SRC_DIR}/SkyBox.cpp
	${SRC_DIR}/TextureManager.cpp
	${SRC_DIR}/Timer.cpp
 	${SRC_DIR}/ImGuiLayer.cpp 
	${SRC_DIR}/DebugUI.cpp "src/Profiler.cpp")

set(IMGUI_SRCS
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_demo.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_impl_dx11.cpp
	${IMGUI_DIR}/imgui_impl_win32.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SRCS} ${IMGUI_SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE 
	${INC_DIR}
	${IMGUI_INC_DIR}
	${XTK_DIR}/Inc
)

target_link_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/${XTK_DIR}/out/lib/Debug>
    $<$<CONFIG:Release,RelWithDebInfo>:${CMAKE_SOURCE_DIR}/${XTK_DIR}/out/lib/Release>

)

target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:RelWithDebInfo>:/DEBUG>
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    DirectXTK
    d3d11
    d3dcompiler
    dxgi
    user32
)

# 전처리기
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:RelWithDebInfo,Debug>:PROFILE>
)
