#pragma once

#include "UHE/Core/Core.h"
#include "UHE/Events/Event.h"
#include <uhepch.h>

namespace UHE {
struct UHE_API WindowProps {
  std::string Title;
  u32 Width;
  u32 Height;

  WindowProps(const std::string &title = "UHE ENGINE", u32 width = 1920,
              u32 height = 1080)
      : Title(title), Width(width), Height(height) {}
};

// interface represented window over the os desktop system
class UHE_API Window {
public:
  using EventCallbackFn = std::function<void(Event &)>;

  virtual ~Window() = default;

  virtual void OnUpdate() = 0;

  virtual u32 GetWidth() const = 0;
  virtual u32 GetHeight() const = 0;

  // Window attributes
  virtual void SetEventCallback(const EventCallbackFn &callback) = 0;
  virtual void SetVSync(bool enabled) = 0;
  virtual bool IsVSync() const = 0;

  // Lock/hide cursor for FPS-style mouse look
  virtual void SetCursorLocked(bool locked) = 0;

  virtual void *GetNativeWindow() const = 0;

  static Window *Create(const WindowProps &props = WindowProps());
};

} // namespace UHE