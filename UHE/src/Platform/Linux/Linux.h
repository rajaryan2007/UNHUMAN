#pragma once
#include "GLFW/glfw3.h"
#include "UHE/Core/Window.h"
#include "UHE/RHI/RHIDevice.h"

namespace UHE::Platform::Linux {
class Linux : public Window {
public:
  Linux(const WindowProps &props);
  virtual ~Linux();
  void OnUpdate() override;

  inline unsigned int GetWidth() const override { return m_Data.Width; }
  inline unsigned int GetHeight() const override { return m_Data.Height; }

  // Window attributes
  inline void SetEventCallback(const EventCallbackFn &callback) override {
    m_Data.EventCallback = callback;
  }
  void SetVSync(bool enabled) override;
  bool IsVSync() const override;

  inline virtual void *GetNativeWindow() const override { return m_Window; }

private:
  virtual void Init(const WindowProps &props);
  virtual void Shutdown();

private:
  GLFWwindow *m_Window;
  RHI::RHIDevice *m_Context;
  struct WindowData {
    std::string Title;
    unsigned int Width, Height;
    bool VSync;
    EventCallbackFn EventCallback;
  };
  WindowData m_Data;
};
} // namespace UHE::Platform::Linux
