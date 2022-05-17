#include "arcpch.h"
#include "Arc/Renderer/OrthographicCameraController.h"

#include "Arc/Core/Input.h"
#include "Arc/Core/KeyCodes.h"
#include "Arc/Core/MouseCodes.h"

namespace ArcEngine
{
	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_AspectRatio(aspectRatio), m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel), m_Rotation(rotation)
	{
	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		ARC_PROFILE_SCOPE();
		
		if(!Input::IsMouseButtonPressed(Mouse::ButtonRight))
			return;

		// Translation
		if(Input::IsKeyPressed(Key::D))
			m_CameraPosition.x += m_CameraTranslationSpeedMultiplier * ts;
		else if(Input::IsKeyPressed(Key::A))
			m_CameraPosition.x -= m_CameraTranslationSpeedMultiplier * ts;

		if(Input::IsKeyPressed(Key::W))
			m_CameraPosition.y += m_CameraTranslationSpeedMultiplier * ts;
		else if(Input::IsKeyPressed(Key::S))
			m_CameraPosition.y -= m_CameraTranslationSpeedMultiplier * ts;

		m_Camera.SetPosition(m_CameraPosition);

		// Rotation
		if(m_Rotation)
		{
			if(Input::IsKeyPressed(Key::Q))
				m_CameraRotation += m_CameraRotationSpeedMultiplier * ts;
			else if(Input::IsKeyPressed(Key::E))
				m_CameraRotation -= m_CameraRotationSpeedMultiplier * ts;

			m_Camera.SetRotation(m_CameraRotation);
		}
	}
	
	void OrthographicCameraController::OnEvent(Event& e)
	{
		ARC_PROFILE_SCOPE();
		
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(ARC_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(ARC_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
	}

	void OrthographicCameraController::OnResize(float width, float height)
	{
		m_AspectRatio = width / height;
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		ARC_PROFILE_SCOPE();
		
		m_ZoomLevel -= e.GetYOffset() * m_ZoomSpeedMultiplier * m_ZoomSpeed;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);

		m_CameraTranslationSpeedMultiplier = m_ZoomLevel * m_CameraTranslationSpeed;
		m_CameraRotationSpeedMultiplier = m_ZoomLevel * m_CameraRotationSpeed;
		
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		ARC_PROFILE_SCOPE();

		OnResize((float)e.GetWidth(), (float)e.GetHeight());
		return false;
	}
}
