cmake_minimum_required(VERSION 3.8)
project(vge LANGUAGES CXX C)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vge)

option(BUILD_EXE "Build as executable (for testing)" 1)

# GLOB_RECURSE is okay for now, just remember to rerun cmake when you add a new file
file(GLOB_RECURSE SRC_FILES "src/*.cpp" "src/*.h")

if(BUILD_EXE)
file(GLOB_RECURSE TEST_FILES "test/*.cpp" "test/*.h")
list(APPEND SRC_FILES ${TEST_FILES})
endif()

add_subdirectory(glm)
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_INSTALL "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
add_subdirectory(glfw)

find_package(Vulkan REQUIRED)

if (WIN32)
   set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif (UNIX)
	set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_X11_KHR)
else()
message(SEND_ERROR "TODO: Linux Mac")
endif()
add_subdirectory(volk)

if(BUILD_EXE)
add_executable(vge ${SRC_FILES})
else()
add_library(vge ${SRC_FILES})
endif()

target_compile_features(vge PRIVATE cxx_std_20)
target_link_libraries(vge PRIVATE volk)
target_link_libraries(vge PRIVATE glm)
target_include_directories(vge PRIVATE ${Vulkan_INCLUDE_DIRS})
target_include_directories(vge PRIVATE tiny_gltf)
target_include_directories(vge PRIVATE "glfw/include")
target_link_libraries(vge PRIVATE glfw)
