# Sandbox Executable

file(GLOB_RECURSE SANDBOX_SOURCES 
    "src/*.h" 
    "src/*.cpp"
)

add_executable(Sandbox ${SANDBOX_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "" FILES ${SANDBOX_SOURCES})

target_include_directories(Sandbox PRIVATE 
    "src"
    "${CMAKE_SOURCE_DIR}/UHE/src"
    "${CMAKE_SOURCE_DIR}/UHE/vendor"
    "${CMAKE_SOURCE_DIR}/UHE/vendor/glm"
    "${CMAKE_SOURCE_DIR}/UHE/vendor/entt/include"
    "${CMAKE_SOURCE_DIR}/UHE/vendor/spdlog/include"
)

target_link_libraries(Sandbox PRIVATE UHE)

if(WIN32)
    target_compile_definitions(Sandbox PRIVATE UHE_PLATFORM_WINDOWS UHE_DYNAMIC_LINK ENTT_API_IMPORT)
elseif(UNIX)
    target_compile_definitions(Sandbox PRIVATE UHE_PLATFORM_LINUX ENTT_API_IMPORT)
endif()

if(UHE_PROFILE)
    target_compile_definitions(Sandbox PRIVATE TRACY_ENABLE=1 TRACY_IMPORTS=1)
endif()

if(MSVC)
    target_compile_options(Sandbox PRIVATE /utf-8)
endif()
