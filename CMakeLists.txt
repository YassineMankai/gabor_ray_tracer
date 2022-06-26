cmake_minimum_required(VERSION 3.9)

project(MyRenderer LANGUAGES CXX)

find_package(OpenMP REQUIRED)

add_subdirectory(External)

add_executable (
	MyRenderer
	Sources/Scene.h
	Sources/Main.cpp
	Sources/Console.h
	Sources/Console.cpp
	Sources/Error.h
	Sources/Error.cpp
	Sources/Image.h
	Sources/Transform.h
	Sources/Camera.h
	Sources/Camera.cpp
	Sources/Mesh.h
	Sources/Mesh.cpp
	Sources/MeshLoader.h
	Sources/MeshLoader.cpp
	Sources/LightSource.h
	Sources/Model.h
	Sources/Ray.h
	Sources/Ray.cpp
	Sources/PBR.h
	Sources/RayTracer.h
	Sources/RayTracer.cpp
	Sources/Rasterizer.h
	Sources/Rasterizer.cpp
	Sources/Resources.h
	Sources/ShaderProgram.h
	Sources/ShaderProgram.cpp
	Sources/Material.h
	Sources/Material.cpp
	Sources/Texture2Dnoise.h
	Sources/Texture2Dnoise.cpp
	Sources/SetupFreeNoise.h
	Sources/SetupFreeNoise.cpp
	Sources/Solid3DNoise.h
	Sources/Solid3DNoise.cpp
	Sources/Texture.h
	Sources/Texture.cpp
	Sources/IO.h
	Sources/IO.cpp
	Sources/BoundingBox.h
	Sources/BoundingBox.cpp
	Sources/BVH.h
	Sources/BVH.cpp
	Sources/Scene.cpp
)

set_target_properties(MyRenderer PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Copy the shader files in the binary location.

add_custom_command(TARGET MyRenderer 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:MyRenderer> ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} External/stb_image/)

target_link_libraries(MyRenderer LINK_PRIVATE glad)

target_link_libraries(MyRenderer LINK_PRIVATE glfw)

target_link_libraries(MyRenderer LINK_PRIVATE glm)

target_link_libraries(MyRenderer PRIVATE OpenMP::OpenMP_CXX)





