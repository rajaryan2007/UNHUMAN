#pragma once
#include <cstdint>
#include <memory>

#ifdef VG_PLATFORM_WINDOWS
#ifdef VG_DYNAMIC_LINK
#ifdef VG_BUILD_DLL
#define UHE_API __declspec(dllexport)
#else
#define UHE_API __declspec(dllimport)
#endif
#else
#define UHE_API
#endif
#elif defined(VG_PLATFORM_LINUX)
#define UHE_API __attribute__((visibility("default")))
#else
#error UHE currently only supports Windows and Linux!
#endif

#ifdef VG_DEBUG
#define VG_ENABLE_ASSERTS
#endif

#ifdef VG_ENABLE_ASSERTS
#ifdef VG_PLATFORM_WINDOWS
#define UHE_DEBUGBREAK() __debugbreak()
#else
#define UHE_DEBUGBREAK() __builtin_trap()
#endif
#define VG_ASSERT(x, ...)                                                      \
  {                                                                            \
    if (!(x)) {                                                                \
      VG_ERROR("Assertion Failed: {0}", __VA_ARGS__);                          \
      UHE_DEBUGBREAK();                                                        \
    }                                                                          \
  }
#define VG_CORE_ASSERT(x, ...)                                                 \
  {                                                                            \
    if (!(x)) {                                                                \
      VG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__);                     \
      UHE_DEBUGBREAK();                                                        \
    }                                                                          \
  }
#else
#define VG_ASSERT(x, ...)
#define VG_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define VG_BIND_EVENT_FN(fn)                                                   \
  [this](auto &&...args) -> decltype(auto) {                                   \
    return this->fn(std::forward<decltype(args)>(args)...);                    \
  }

namespace UHE {

template <typename T> using Scope = std::unique_ptr<T>;

template <typename T, typename... Args>
constexpr Scope<T> CreateScope(Args &&...args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T> using Ref = std::shared_ptr<T>;

template <typename T, typename... Args>
constexpr Ref<T> CreateRef(Args &&...args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T> using WeakRef = std::weak_ptr<T>;

template <typename T, typename... Args>
constexpr WeakRef<T> CreateWeakRef(Args &&...args) {
  return std::shared_ptr<T>(std::forward<Args>(args)...);
}
} // namespace UHE
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;
