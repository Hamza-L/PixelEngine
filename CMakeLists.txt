cmake_minimum_required(VERSION 3.17)

project(PixelEngine)
set(PROJECT_NAME PixelEngine)
set(CMAKE_CXX_STANDARD 17)

if (WIN32)
    message("Starting the Windows Build")
endif (WIN32)

if (UNIX AND APPLE)
    message("Starting the MacOS Build")
endif (UNIX AND APPLE)


################################################################################
# Source groups
################################################################################
set(Headers
    "source/PixelRenderer.h"
    "source/PixelWindow.h"
)
source_group("Headers" FILES ${Headers})

set(Sources
    "source/main.cpp"
    "source/PixelRenderer.cpp"
    "source/PixelWindow.cpp"
)
source_group("Sources" FILES ${Sources})

set(ALL_FILES
    ${Headers}
    ${Sources}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

set(ROOT_NAMESPACE PixelEngine)

set(CMAKE_VS_PLATFORM_NAME "x64")

set(VULKAN_SDK "/Users/hamzalah/VulkanSDK/1.3.239.0/macOS")

################################################################################
# Include directories
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/external/GLFW64/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/GLM;"
        "${VULKAN_SDK}/include"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/external/GLFW/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/GLM;"
        "${VULKAN_SDK}/include"
    )
endif()

################################################################################
# Dependencies
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "libvulkan.1.2.162.dylib"
        "libvulkan.dylib"
        "glfw3"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "vulkan-1"
        "glfw3"
    )
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_link_directories(${PROJECT_NAME} PRIVATE
        "${VULKAN_SDK}/lib/libvulkan.1.2.162.dylib"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/GLFW64/lib-vc2022"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_link_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/external/GLFW/lib-vc2022"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/GLFW/lib-vc2022"
        "C:/VulkanSDK/1.3.239.0/Lib32"
    )
endif()

