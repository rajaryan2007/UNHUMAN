#include "uhepch.h"
#include "OrthograpicCameraContoroller.h"
#include "UHE/Renderer2D/OrthographicCamera.h"
#include "UHE/Core/Timestep.h"
#include "UHE/Core/keyCodes.h"
#include "UHE/Core/input.h"

// Fix constructor initialization for m_Camera and m_Bounds
UHE::OrthographicCameraContoroller::OrthographicCameraContoroller(f32 aspectRatio, bool rotation /*= false*/)
	: m_AspectRatio(aspectRatio), m_Rotation(rotation), m_ZoomLevel(1.0f),
	m_Bounds{ -aspectRatio * m_ZoomLevel, aspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel },
	m_Camera(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top)
{
	// Other initialization if needed
}

void UHE::OrthographicCameraContoroller::OnUpdate(Timestep ts)
{
	m_CameraTranslationSpeed = m_ZoomLevel;
	if (Input::IsKeyPressed(UHE_KEY_LEFT))
	{
		m_CameraPosition.x += m_CameraTranslationSpeed * ts;
	}
	else if (Input::IsKeyPressed(UHE_KEY_RIGHT))
	{
		m_CameraPosition.x -= m_CameraTranslationSpeed * ts;
	}
	if (Input::IsKeyPressed(UHE_KEY_DOWN))
	{
		m_CameraPosition.y += m_CameraTranslationSpeed * ts;
	}
	else if (Input::IsKeyPressed(UHE_KEY_UP))
	{
		m_CameraPosition.y -= m_CameraTranslationSpeed * ts;
	}

	

	if (m_Rotation) {

		if (Input::IsKeyPressed(UHE_KEY_A))
		{
			m_CameraRotation -= m_CameraRotationSpeed * ts;
		}
		else if (Input::IsKeyPressed(UHE_KEY_D))
		{
			m_CameraRotation += m_CameraRotationSpeed * ts;
		}

		if (Input::IsKeyPressed(UHE_KEY_J))
		{
			m_CameraRotation += m_CameraRotationSpeed * ts;
		}
		else if (Input::IsKeyPressed(UHE_KEY_L))
		{
			m_CameraRotation -= m_CameraRotationSpeed * ts;
		}
		if (Input::IsKeyPressed(UHE_KEY_I))
		{
			m_CameraRotation += m_CameraRotationSpeed * ts;
		}
		else if (Input::IsKeyPressed(UHE_KEY_K))
		{
			m_CameraRotation -= m_CameraRotationSpeed * ts;
		}
		m_Camera.SetRotation(m_CameraRotation);
	}
	m_Camera.SetPosition(m_CameraPosition);

}

void UHE::OrthographicCameraContoroller::OnEvent(Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<MouseScrolledEvent>(UHE_BIND_EVENT_FN(OrthographicCameraContoroller::OnMouseScrolled));
	dispatcher.Dispatch<WindowResizeEvent>(UHE_BIND_EVENT_FN(OrthographicCameraContoroller::OnWindowResized));
}

void UHE::OrthographicCameraContoroller::OnResize(f32 width, u32 height)
{
	m_AspectRatio = width / (f32)height;
	CalculateView();
}


void UHE::OrthographicCameraContoroller::CalculateView()
{
	m_Bounds = { -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel };
	m_Camera.SetProjection(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top);
}

bool UHE::OrthographicCameraContoroller::OnMouseScrolled(MouseScrolledEvent& e)
{
	m_ZoomLevel -= e.GetYOffset() * 0.25f;
	m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
	//m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	CalculateView();
	return false;
}

bool UHE::OrthographicCameraContoroller::OnWindowResized(WindowResizeEvent& e)
{
	m_AspectRatio = (f32)e.GetWidth() / (f32)e.GetHeight();
	CalculateView();

	//m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	return false;
}
