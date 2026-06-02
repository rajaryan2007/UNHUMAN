# UHE Shared Library
project(UHE)



file(GLOB_RECURSE UHE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image/*.cpp"
)

find_package(Vulkan REQUIRED)
if(WIN32)
    set(SLANG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/bin/windows/slang")
else()
    set(SLANG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/bin/linux/slang")
endif()
find_package(slang REQUIRED CONFIG PATHS "${SLANG_DIR}" NO_DEFAULT_PATH)

file(GLOB_RECURSE GLM_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm/glm/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm/glm/*.inl"
)

set(ALL_UHE_FILES ${UHE_SOURCES} ${GLM_FILES})


add_library(UHE SHARED ${ALL_UHE_FILES}  )


source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "" FILES ${ALL_UHE_FILES})


target_include_directories(UHE PUBLIC
    "src"
    "vendor/spdlog/include"
    "vendor/GLFW/include"
    "vendor/Glad/include"
    "vendor/imgui"
    "vendor/imGuizmo"
    "vendor/glm"
    "vendor/stb_image"
    "vendor/entt/include"
    "vendor/volk"
    "vendor/VulkanMemoryAllocator/include"
    "vendor/yaml-cpp/include"
    "vendor/tracy/public"
    "vendor/slang/include"
    "vendor/box2d/include"
)

target_sources(UHE PRIVATE
    vendor/tracy/public/TracyClient.cpp
)

if(WIN32)
    target_compile_definitions(UHE PUBLIC VG_PLATFORM_WINDOWS)
    target_compile_definitions(UHE PUBLIC VG_DYNAMIC_LINK WINDLL)
elseif(UNIX)
    target_compile_definitions(UHE PUBLIC VG_PLATFORM_LINUX)
    # UNIX doesn't typically need WINDLL or explicit dllexport macros for shared objects if visibility is handled
endif()

target_compile_definitions(UHE PUBLIC
    GLFW_INCLUDE_NONE
    _CRT_SECURE_NO_WARNINGS
)

target_compile_definitions(UHE PRIVATE VG_BUILD_DLL TRACY_ENABLE)

if(VG_PROFILE)
    target_compile_definitions(UHE PUBLIC VG_PROFILE=1 TRACY_ENABLE=1)
    target_compile_definitions(UHE PRIVATE TRACY_EXPORTS=1)
endif()

if(MSVC)
    target_compile_options(UHE PRIVATE /utf-8)
endif()

# Enable precompiled headers for ALL compilers (MSVC, GCC, Clang)
target_precompile_headers(UHE PRIVATE src/uhepch.h)

target_link_libraries(UHE PUBLIC
    glfw Glad Vulkan::Vulkan ImGui ImGuizmo
PRIVATE
    VulkanMemoryAllocator volk
    yaml-cpp
    TracyClient
    slang::slang
    box2d
)

if(WIN32)
    target_link_libraries(UHE PUBLIC opengl32.lib)
endif()

# Copy Slang DLLs to output directory
add_custom_command(TARGET UHE POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${SLANG_DIR}/bin"
    "$<TARGET_FILE_DIR:UHE>"
)
set_target_properties(UHE PROPERTIES DEBUG_POSTFIX d)
