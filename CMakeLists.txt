cmake_minimum_required(VERSION 3.23)
project(Vulkan_L_CLion)

set(CMAKE_CXX_STANDARD 14)

file(GLOB SOURCES "Source_Files/*.cpp")

add_library(Vulkan_L_Lib STATIC IMPORTED)

add_library(Vulkan_L_Lib2 STATIC IMPORTED)

add_library(Vulkan_L_Lib3 STATIC IMPORTED)

#IGNORE--------------------------------------------------
#set_property(TARGET Vulkan_L_Lib2 PROPERTY IMPORTED_LOCATION "C:\\Users\\bhara\\CLionProjects\\Vulkan_L_CLion\\lib\\glfw3.dll")

#set_property(TARGET Vulkan_L_Lib PROPERTY IMPORTED_LOCATION "C:\\Users\\bhara\\CLionProjects\\Vulkan_L_CLion\\lib\\vulkan-1.dll")

#set_property(TARGET Vulkan_L_Lib3 PROPERTY IMPORTED_LOCATION "C:\\Users\\bhara\\CLionProjects\\Vulkan_L_CLion\\lib\\libVkLayer_khronos_validation.dll")
#--------------------------------------------------------

set_property(TARGET Vulkan_L_Lib2 PROPERTY IMPORTED_LOCATION "../lib/glfw3.dll")

set_property(TARGET Vulkan_L_Lib PROPERTY IMPORTED_LOCATION "../lib/vulkan-1.dll")

set_property(TARGET Vulkan_L_Lib3 PROPERTY IMPORTED_LOCATION "../lib/libVkLayer_khronos_validation.dll")

add_executable(Vulkan_L_CLion ${SOURCES} Headers/lve_computepipeline.hpp Source_Files/lve_computepipeline.cpp)

target_include_directories(Vulkan_L_CLion PUBLIC Headers ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/vulkan glm glfw-master/include/GLFW/)

target_link_libraries(Vulkan_L_CLion PUBLIC Vulkan_L_Lib3 Vulkan_L_Lib2 Vulkan_L_Lib -static gcc stdc++ winpthread -dynamic)