#pragma once
#include "UHE/Renderer/GraphicsContext.h"
#include <vulkan/vulkan_raii.hpp>

struct GLFWwindow;
namespace UHE {
 

   class VkContext : public GraphicsContext {
   public:
     VkContext(GLFWwindow *windowHandle);
     virtual void Init() override;
     virtual void SwapBuffers() override;
   
   private:
     GLFWwindow *m_WindowHandle;
   };
}	

