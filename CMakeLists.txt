cmake_minimum_required(VERSION 3.21)

set(PROJECT_NAME VoxelWorld)

project(${PROJECT_NAME} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC_DIR src)
set(INC_DIR includes)
set(THIRD_PART_DIR third_party)
set(IMGUI_DIR ${THIRD_PART_DIR}/imgui_linux)
set(IMGUI_INC_DIR ${THIRD_PART_DIR}/imgui_linux)
set(GLFW_DIR ${THIRD_PART_DIR}/glfw)
set(GLM_DIR ${THIRD_PART_DIR}/glm)
set(GLAD_DIR ${THIRD_PART_DIR}/glad)

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
	${SRC_DIR}/Platform.cpp
	${SRC_DIR}/Renderer.cpp
	${SRC_DIR}/RuntimeConfig.cpp
	${SRC_DIR}/ScreenManager.cpp
	${SRC_DIR}/SkyBox.cpp
	${SRC_DIR}/TextureManager.cpp
	${SRC_DIR}/Timer.cpp
 	${SRC_DIR}/ImGuiLayer.cpp 
	${SRC_DIR}/DebugUI.cpp 
	${SRC_DIR}/Profiler.cpp
	${SRC_DIR}/JobScheduler.cpp)

set(IMGUI_SRCS
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_demo.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_impl_glfw.cpp
	${IMGUI_DIR}/imgui_impl_opengl3.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
)

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/${GLFW_DIR}/CMakeLists.txt)
	message(FATAL_ERROR "GLFW was not found. Run installer/Linux/install_glfw.sh first.")
endif()

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/${GLM_DIR}/glm/glm.hpp)
	message(FATAL_ERROR "GLM was not found. Run installer/Linux/install_glm.sh first.")
endif()

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/${GLAD_DIR}/cmake/CMakeLists.txt)
	message(FATAL_ERROR "glad was not found. Run installer/Linux/install_opengl.sh first.")
endif()

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/${IMGUI_DIR}/imgui_impl_glfw.cpp)
	message(FATAL_ERROR "ImGui GLFW/OpenGL3 backend was not found. Install imgui with glfw + opengl3.")
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${GLFW_DIR})
add_subdirectory(${GLAD_DIR}/cmake glad_cmake)

find_package(OpenGL REQUIRED)
glad_add_library(glad_gl_core_33 REPRODUCIBLE API gl:core=3.3)

add_executable(${PROJECT_NAME} ${SRCS} ${IMGUI_SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE 
	${INC_DIR}
	${IMGUI_INC_DIR}
	${GLM_DIR}
)

target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:RelWithDebInfo>:/DEBUG>
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glad_gl_core_33
    glfw
    OpenGL::GL
)

# 전처리기
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:RelWithDebInfo,Debug>:PROFILE>
)

option(ENABLE_AUTOMATION_TEST "Enable automation test" OFF)

if(ENABLE_AUTOMATION_TEST)
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
		ENABLE_AUTOMATION_TEST=1
	)
endif()


