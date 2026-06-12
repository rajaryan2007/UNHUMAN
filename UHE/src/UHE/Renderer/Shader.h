#pragma once
#include "UHE/Core/Core.h"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace UHE {
class UHE_API Shader {
public:
  virtual ~Shader() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;


  virtual const std::string &GetName() const = 0;

  static Ref<Shader> Create(const std::string &filepath);
  static Ref<Shader> Create(const std::string &name,
                            const std::string &vertexSrc,
                            const std::string &fragmentSrc);

private:
  u32 m_RendererID;
};

class UHE_API ShaderLibrary {
public:
  void Add(const Ref<Shader> &shader);
  void Add(const std::string &name, const Ref<Shader> &shader);

  Ref<Shader> Load(const std::string &filepath);
  Ref<Shader> Load(const std::string &name, const std::string &filepath);

  Ref<Shader> Get(const std::string &name);
 
private:
  std::unordered_map<std::string, Ref<Shader>> m_Shaders;
  
};
} // namespace UHE