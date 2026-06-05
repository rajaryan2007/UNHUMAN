#include "WindowsWindow.h"
#include "UHE/AssestsManager/VfsSystem.h"
#include "UHE/Core/Log.h"
#include "UHE/Events/ApplicationEvent.h"
#include "UHE/Events/KeyEvent.h"
#include "UHE/Events/MouseEvent.h"
#include "backends/imgui_impl_glfw.h"
#include "glad/glad.h"
#include "imgui.h"
#include "uhepch.h"

#include "Platform/OpenGL/OpenGLContext.h"
#include <stb_image.h>

namespace UHE {
static bool s_GLFWInitialized = false;

static void GLFWErrorCallback(int error, const char *descrioption) {
  VG_CORE_ERROR("GLFW_ERROR({0}):{1}", error, descrioption);
}

Window *Window::Create(const WindowProps &props) {
  UHE_PROFILE_FUNCTION();
  return new WindowsWindow(props);
}
WindowsWindow::WindowsWindow(const WindowProps &props) {
  UHE_PROFILE_FUNCTION();
  Init(props);
}
WindowsWindow::~WindowsWindow() { Shutdown(); }

void WindowsWindow::Init(const WindowProps &props) {
  m_Data.Title = props.Title;
  m_Data.Width = props.Width;
  m_Data.Height = props.Height;

  VG_CORE_INFO("CREATE WINDOW {0} : {1}, {2}", props.Title, props.Width,
               props.Height);

  if (!s_GLFWInitialized) {
    int success = glfwInit();
    VG_CORE_ASSERT(success, "Could not initialize GLFW!");
    glfwSetErrorCallback(GLFWErrorCallback);
    s_GLFWInitialized = true;
  }

  // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Breaks mouse dragging/events
  // without custom Win32 event hooks!
  m_Window = glfwCreateWindow((int)props.Width, (int)props.Height,
                              m_Data.Title.c_str(), nullptr, nullptr);
  GLFWimage images[1];
  const std::string path = UHE::FileSystem::Get().Resolve("icon\\ace.jpg");

  images[0].pixels =
      stbi_load(path.c_str(), &images[0].width, &images[0].height, 0, 4);
  if (images[0].pixels) {
    glfwSetWindowIcon(m_Window, 1, images);
    stbi_image_free(images[0].pixels);
  } else {
    images[0].pixels =
        stbi_load(path.c_str(), &images[0].width, &images[0].height, 0, 4);
    if (images[0].pixels) {
      glfwSetWindowIcon(m_Window, 1, images);
      stbi_image_free(images[0].pixels);
    } else {
      VG_CORE_WARN("Could not load window icon.");
    }
  }

  m_Context = new OpenGLContext(m_Window);
  m_Context->Init();
  // glfwMakeContextCurrent(m_Window);
  // int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  // VG_CORE_ASSERT(status, "Failed to initialize Glad!");
  glfwSetWindowUserPointer(m_Window, &m_Data);
  SetVSync(true);

  // If ImGui backend installed callbacks earlier with install_callbacks=true,
  // it saved previous callbacks and chained them. When using
  // install_callbacks=false we must forward the callbacks manually; ensure
  // backend data is initialized by setting current context.

  // set GLFW callbacks here
  glfwSetWindowSizeCallback(
      m_Window, [](GLFWwindow *window, int width, int height) {
        WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);

        WindowResizeEvent event(width, height);
        data.Width = width;
        data.Height = height;
        data.EventCallback(event);
      });
  glfwSetWindowCloseCallback(m_Window, [](GLFWwindow *window) {
    WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
    WindowCloseEvent event;
    data.EventCallback(event);
  });
  glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int key, int scancode,
                                  int action, int mods) {
    WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
    switch (action) {
    case GLFW_PRESS: {
      KeyPressedEvent event(key, 0);
      data.EventCallback(event);
      break;
    }
    case GLFW_RELEASE: {
      KeyReleasedEvent event(key);
      data.EventCallback(event);
      break;
    }
    case GLFW_REPEAT: {
      KeyPressedEvent event(key, 1);
      data.EventCallback(event);
      break;
    }
    }
  });
  glfwSetCharCallback(m_Window, [](GLFWwindow *window, unsigned int keycode) {
    WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);

    KeyTypedEvent event(keycode);
    data.EventCallback(event);
  });

  glfwSetMouseButtonCallback(
      m_Window, [](GLFWwindow *window, int button, int action, int mods) {
        WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);

        switch (action) {
        case GLFW_PRESS: {
          MouseButtonPressedEvent event(button);
          data.EventCallback(event);
          break;
        }
        case GLFW_RELEASE: {
          MouseButtonReleasedEvent event(button);
          data.EventCallback(event);
          break;
        }
        }
      });

  glfwSetScrollCallback(
      m_Window, [](GLFWwindow *window, double xOffset, double yOffset) {
        WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);

        MouseScrolledEvent event((float)xOffset, (float)yOffset);
        data.EventCallback(event);
      });

  glfwSetCursorPosCallback(
      m_Window, [](GLFWwindow *window, double xPos, double yPos) {
        WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);

        MouseMovedEvent event((float)xPos, (float)yPos);
        data.EventCallback(event);
      });
}

void WindowsWindow::Shutdown() {
  UHE_PROFILE_FUNCTION();
  glfwDestroyWindow(m_Window);
  /*--s_GLFWindowCount;
  if(s_GLFWindowCount == 0) {
          glfwTerminate();
  }*/
}

void WindowsWindow::OnUpdate() {
  // Poll events and swap buffers

  glfwPollEvents();
  m_Context->SwapBuffers();
  // glfwSwapBuffers(m_Window);
}

void WindowsWindow::SetVSync(bool enabled) {
  if (enabled)
    glfwSwapInterval(1);
  else
    glfwSwapInterval(0);
  m_Data.VSync = enabled;
}

bool WindowsWindow::IsVSync() const { return m_Data.VSync; }

} // namespace UHE
