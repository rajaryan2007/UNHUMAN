cmake_minimum_required(VERSION 3.16)
project(UHE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Make RPATH/RUNPATH relative to the executable using $ORIGIN
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "$ORIGIN")


add_compile_definitions(ENTT_STANDARD_CPP)
add_compile_definitions(VULKAN_HPP_NO_STRUCT_CONSTRUCTORS)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Setup submodules automatically

option(VG_PROFILE "Enable Tracy profiling" ON)

option(UHE_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(UHE_ENABLE_MSAN "Enable MemorySanitizer" OFF)

if(UHE_ENABLE_ASAN AND UHE_ENABLE_MSAN)
    message(FATAL_ERROR "AddressSanitizer and MemorySanitizer cannot be enabled simultaneously.")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    if (UHE_ENABLE_ASAN)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (UHE_ENABLE_MSAN)
        add_compile_options(-fsanitize=memory -fno-omit-frame-pointer -fPIE)
        add_link_options(-fsanitize=memory -pie)
    endif()
elseif(UHE_ENABLE_MSAN)
    message(FATAL_ERROR "MemorySanitizer is only supported with upstream Clang.")
endif()

set(ARCH x64)
string(TOLOWER ${CMAKE_SYSTEM_NAME} SYS_NAME)
if(CMAKE_BUILD_TYPE)
  set(OUTPUT_DIR "${CMAKE_BUILD_TYPE}-${SYS_NAME}-${ARCH}")
else()
  set(OUTPUT_DIR "Debug-${SYS_NAME}-${ARCH}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${OUTPUT_DIR})

foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${CONFIG} CONFIG_UPPER)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_SOURCE_DIR}/bin/${CONFIG}-${SYS_NAME}-${ARCH})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_SOURCE_DIR}/bin/${CONFIG}-${SYS_NAME}-${ARCH})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_SOURCE_DIR}/bin/${CONFIG}-${SYS_NAME}-${ARCH})
endforeach()


function(auto_group target)
  get_target_property(files ${target} SOURCES)
  foreach(file ${files})
    file(RELATIVE_PATH rel ${CMAKE_SOURCE_DIR} ${file})
    get_filename_component(dir ${rel} PATH)
    string(REPLACE "/" "\\" group ${dir})
    source_group("${group}" FILES ${file})
  endforeach()
endfunction()

# all libs
add_subdirectory(UHE/vendor/GLFW)

# ImGui is built here instead of add_subdirectory(UHE/vendor/imgui): the vendored
# fork's CMakeLists.txt defines IMGUI_API=__declspec(dllexport) unconditionally,
# which breaks GCC/Linux builds
add_library(ImGui STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/UHE/vendor/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UHE/vendor/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UHE/vendor/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UHE/vendor/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UHE/vendor/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UHE/vendor/imgui/misc/cpp/imgui_stdlib.cpp
)
if(MSVC)
    target_compile_definitions(ImGui PRIVATE IMGUI_API=__declspec\(dllexport\))
endif()
target_include_directories(ImGui PUBLIC UHE/vendor/imgui)

add_subdirectory(UHE/vendor/imGuizmo)
add_subdirectory(UHE/vendor/volk)
add_subdirectory(UHE/vendor/VulkanMemoryAllocator)
add_subdirectory(UHE/vendor/yaml-cpp)
add_subdirectory(UHE/vendor/box2d)
add_subdirectory(UHE/vendor/fastgltf)

# MSDF text rendering (msdf-atlas-gen + msdfgen)
set(MSDF_ATLAS_USE_VCPKG OFF        CACHE BOOL "" FORCE)
set(MSDF_ATLAS_USE_SKIA  OFF        CACHE BOOL "" FORCE)
set(MSDF_ATLAS_BUILD_STANDALONE OFF CACHE BOOL "" FORCE)
set(MSDF_ATLAS_NO_ARTERY_FONT ON    CACHE BOOL "" FORCE)
add_subdirectory(UHE/vendor/msdf-atlas-gen)

# Disable Jolt Physics tests and viewers to drastically reduce compile times
set(TARGET_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(TARGET_HELLO_WORLD OFF CACHE BOOL "" FORCE)
set(TARGET_PERFORMANCE_TEST OFF CACHE BOOL "" FORCE)
set(TARGET_SAMPLES OFF CACHE BOOL "" FORCE)
set(TARGET_VIEWER OFF CACHE BOOL "" FORCE)
set(USE_SSE4_2 ON CACHE BOOL "" FORCE) # Fast SIMD physics
set(USE_AVX2 OFF CACHE BOOL "" FORCE)  # Keep off for broader compatibility
set(USE_AVX512 OFF CACHE BOOL "" FORCE)
set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE) # Force Jolt to use /MD instead of /MT
add_subdirectory(UHE/vendor/jolt/Build)
#Tracy Profiler
set(TRACY_STATIC ON CACHE BOOL "" FORCE)
set(TRACY_ENABLE ${VG_PROFILE} CACHE BOOL "" FORCE)
add_subdirectory(UHE/vendor/tracy)

#main engine core
add_subdirectory(UHE)
#sandbox
add_subdirectory(sandbox)
add_subdirectory(UHE_EDITOR)
add_subdirectory(UHEGAME)


get_property(all_targets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
foreach(t ${all_targets})
  auto_group(${t})
endforeach()

# Add 'make format' target
add_custom_target(format
    COMMAND find ${CMAKE_SOURCE_DIR}/UHE/src ${CMAKE_SOURCE_DIR}/UHE_EDITOR/src -type f \\( -name "*.cpp" -o -name "*.h" \\) -exec clang-format -i {} +
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Formatting source files with clang-format"
)
