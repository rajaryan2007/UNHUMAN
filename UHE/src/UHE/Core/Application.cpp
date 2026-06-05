#include "uhepch.h"

#include "Application.h"

#include "UHE/Renderer/Renderer.h"
#include "UHE/Debug/MemoryProfile.h"

#include "UHE/Core/Log.h"
#include "input.h"

#include <GLFW/glfw3.h>

namespace UHE{

#define BIND_EVENT_FN(x) std::bind(&Application::x,this,std::placeholders::_1)

	Application* Application::s_instance = nullptr;

	

	Application::Application( const std::string& name)
		
	{   
		UHE_PROFILE_FUNCTION();
		VG_CORE_ASSERT(!s_instance, "Application already exists!");
		
		s_instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create(WindowProps(name)));
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		m_Window->SetVSync(false);
		
		Renderer::Init();
		Debug::RunMemoryProfilingSmokeTest();
		Debug::RunGpuProfilingSmokeTest();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);		
	}
	 
	Application::~Application()
	{

	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::Close()
	{
		m_Running = false;
	}

	//bool Application::OnWindowResize(WindowResizeEvent& e)
	//{
	//	// Update the OpenGL viewport immediately
	//	glViewport(0, 0, (int)e.GetWidth(), (int)e.GetHeight());
	//	VG_CORE_INFO("Window resized: {0}, {1}", e.GetWidth(), e.GetHeight());
	//	// return false so other layers may also handle the event if needed
	//	return false;
	//}
	
	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize)); // <-- add dispatch

		//VG_CORE_TRACE("{0}", e.ToString());

		
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	};
	
	/*OnWindowResize(WindowResizeEvent& e);*/
	
	void Application::Run() {
    // Dispatch an initial resize event to set up the viewport and camera
    {
			
        int width = m_Window->GetWidth();
        int height = m_Window->GetHeight();
        WindowResizeEvent e(width, height);
        OnEvent(e);
    }

	while (m_Running)
	{   
		UHE_PROFILE_SCOPE("Frame");
		{
			UHE_PROFILE_SCOPE("application");
			float time = (float)glfwGetTime();

			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (timestep > 0.25f)
				timestep = 0.25f;

			if (!m_Minimized) {
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(timestep);

			}
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();


			m_Window->OnUpdate();
		}
		UHE_PROFILE_FRAMEMARK;
	}

	};
	bool Application::OnWindowClose(WindowCloseEvent& e) 
	{   
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if(e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
		return false;
	}
}
