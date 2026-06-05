#include "uhepch.h"
#include "backends/imgui_impl_glfw.cpp"

#ifdef UHE_RENDERER_OPENGL
    #define IMGUI_IMPL_OPENGL_LOADER_GLAD
    #include "backends/imgui_impl_opengl3.cpp"
#endif

#ifdef UHE_RENDERER_VULKAN
    #include "backends/imgui_impl_vulkan.cpp"
#endif
