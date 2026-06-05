#include "uhepch.h"
#include "UHE\Renderer2D\Renderer2D.h"
#include "Renderer.h"
#include "RendererAPI.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include "Shader.h"
namespace UHE {

	Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;

	void Renderer::Init() {
		UHE_PROFILE_FUNCTION();
		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::OnWindowResize(u32 width, u32 height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(OrthographicCamera& camera) {
        // Implementation for beginning a scene
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

    void Renderer::EndScene() 
	{
        // Implementation for ending a scene
	}

    void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4 transform ) {
        // Implementation for submitting a vertex array to be rendered
		
		shader->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader)->SetUniformMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->SetUniformMat4("u_Transform", transform);
		
		/*mi.bind();*/

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
	

}