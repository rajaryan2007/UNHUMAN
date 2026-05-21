#pragma once




namespace UHE::RHI {
class RHIShader {
public:
  virtual ~RHIShader() = default;

  void bind();
  void Unbind();

  
  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual void SetFloat3(const std::string &name, const glm::vec3 &value) = 0;
  virtual void SetIntArray(const std::string &name, int *values, u32 count) = 0;
  virtual void SetFloat4(const std::string &name, const glm::vec4 &value) = 0;
  virtual void SetMat4(const std::string &name, const glm::mat4 &value) = 0;
  virtual void SetInt(const std::string &name, const int value) = 0;
  virtual void SetFloat(const std::string &name, const float value) = 0;

  virtual const std::string &GetName() const = 0;

  static Ref<RHIShader> Create(const std::string &filepath);
  static Ref<RHIShader> Create(const std::string &name,
                            const std::string &vertexSrc,
                            const std::string &fragmentSrc);

  private:

};

class ShaderLibrary {
public:
    static std::unique_ptr<ShaderLibrary> Create();
    RHIShader* GetShader(const std::string& name);
    void AddShader(const std::string& name, std::unique_ptr<RHIShader> shader);

private:
    ShaderLibrary() = default;
    std::unordered_map<std::string, std::unique_ptr<RHIShader>> m_Shaders;
};