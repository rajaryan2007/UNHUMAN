#include "uhepch.h"
#include "Shader.h"
#include <filesystem>
#include <fstream>
#include "Platform/OpenGL/OpenGLShader.h"
#include "Renderer.h"

namespace fs = std::filesystem;
namespace UHE
{

	Ref<Shader> Shader::Create(const std::string& filepath)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			UHE_CORE_ASSERT(false, "RendererAPI::NONE is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLShader>(filepath);
		}
		UHE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			UHE_CORE_ASSERT(false, "RendererAPI::NONE is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLShader>(name, vertexSrc, fragmentSrc);
		
		}

		UHE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		auto& name = shader->GetName();
		UHE_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end(), "Shader already exists!");
		m_Shaders[name] = shader;

	}

	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
	{
		UHE_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end(), "Shader already exists!");
		m_Shaders[name] = shader;
	}

    UHE::Ref<UHE::Shader> ShaderLibrary::Load(const std::string& filepath)
    {
					auto shader = Shader::Create(filepath);
					Add(shader);
					return shader;
    }

    UHE::Ref<UHE::Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath)
    {
					auto shader = Shader::Create(filepath);
					Add(name, shader);
					return shader;
    }

	//UHE::Ref<UHE::Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath)
	//{
	//	auto shader = Shader::Create(filepath);
	//	Add(name,shader);
	//	return shader;
	//}
	
	UHE::Ref<UHE::Shader> ShaderLibrary::Get(const std::string& name)
	{
		UHE_CORE_ASSERT(m_Shaders.find(name) != m_Shaders.end(), "Shader not found!");
		return m_Shaders[name];
	}

}
