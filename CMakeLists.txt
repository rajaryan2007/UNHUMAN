cmake_minimum_required(VERSION 3.16)
project(UHE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Setup submodules automatically

option(VG_PROFILE "Enable Tracy profiling" ON)

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
add_subdirectory(UHE/vendor/Glad)
add_subdirectory(UHE/vendor/imgui)
add_subdirectory(UHE/vendor/imGuizmo)
add_subdirectory(UHE/vendor/volk)
add_subdirectory(UHE/vendor/VulkanMemoryAllocator)
add_subdirectory(UHE/vendor/yaml-cpp)
add_subdirectory(UHE/vendor/box2d)

#Tracy Profiler
set(TRACY_STATIC ON CACHE BOOL "" FORCE)
set(TRACY_ENABLE ${VG_PROFILE} CACHE BOOL "" FORCE)
add_subdirectory(UHE/vendor/tracy)

#main engine core
add_subdirectory(UHE)
#sandbox
add_subdirectory(sandbox)
add_subdirectory(UHE_EDITOR)


get_property(all_targets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
foreach(t ${all_targets})
    auto_group(${t})
endforeach()
