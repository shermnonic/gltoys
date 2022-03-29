cmake_minimum_required(VERSION 3.1)

project(gltoys 
    VERSION 0.1
    DESCRIPTION "OpenGL toys by 386dx25"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) # This might be insufficient on gcc
include_directories(./)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})
link_directories(${GLM_LIBRARY_DIRS})
add_definitions(${GLM_DEFINITIONS})

find_package(imgui CONFIG REQUIRED) # consider making imgui optional
set(imgui-impl-sources
  3rdparty/imgui/imgui_impl_glfw.h
  3rdparty/imgui/imgui_impl_glfw.cpp
  3rdparty/imgui/imgui_impl_opengl3.h
  3rdparty/imgui/imgui_impl_opengl3.cpp
)
add_compile_definitions(IMGUI_IMPL_OPENGL_LOADER_GLEW)

set(glutils-sources
  glutils/GLConfig.h
  glutils/GLError.h
  glutils/GLError.cpp
  glutils/GLSLProgram.h
  glutils/GLSLProgram.cpp
  glutils/GLTexture.h
  glutils/GLTexture.cpp
  glutils/Trackball2.h
  glutils/Trackball2.cpp
  glutils/Frustum.h
  glutils/Frustum.cpp
  glutils/Octree.h
  glutils/MeshBufferTypes.h
  glutils/MeshBuffer.h
  glutils/MeshBuffer.cpp
  glutils/MeshBufferIO.h
  glutils/MeshBufferIO.cpp
  glutils/MeshShader.h
  glutils/GLMeshObject.h
  glutils/GLMeshObject.cpp
)

set(fx-sources
  fx/PerlinNoise.h
  fx/PerlinNoise.cpp
  fx/MarchingCubes.h
  fx/MarchingCubes.cpp
)

set(utils-sources
  utils/ComputeThreads.h
)

set(params-sources
  utils/ParameterBase.h
  utils/ParameterBase.cpp
  utils/ParameterIO.h
  utils/ParameterIO.cpp
  utils/ParameterTypes.h
)

source_group(glutils FILES ${glutils-sources})
source_group(imgui-impl FILES ${imgui-impl-sources})
source_group(fx FILES ${fx-sources})
source_group(utils FILES ${utils-sources})
source_group(utils FILES ${params-sources})

include_directories(
  3rdparty
  ${GLEW_INCLUDE_DIR}
  ${GLFW_INCLUDE_DIR}
)

add_library(toylib STATIC 
    ${glutils-sources} 
    ${fx-sources}
    ${imgui-impl-sources}
    ${utils-sources}
)

target_link_libraries(toylib
  imgui::imgui
  GLEW::GLEW
  glfw
  ${OPENGL_LIBRARY}
)

add_executable(toy-mnoise toy-mnoise.cpp GLFWApp.h GLFWApp.cpp MCubesObject.h MCubesObject.cpp)
target_link_libraries(toy-mnoise toylib)

add_executable(toy-hello toy-hello.cpp)
target_link_libraries(toy-hello toylib)

add_executable(toy-glitchsphere toy-glitchsphere.cpp GLFWApp.h GLFWApp.cpp GlitchSphereGeometry.h GlitchSphereGeometry.cpp)
target_link_libraries(toy-glitchsphere toylib)

find_package (Threads)
add_executable(test-threads test-threads.cpp)
target_link_libraries (test-threads ${CMAKE_THREAD_LIBS_INIT})

add_executable(test-params test-params.cpp ${params-sources})
