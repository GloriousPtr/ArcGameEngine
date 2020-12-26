#pragma once

#include "Arc/Renderer/OrthographicCamera.h"
#include "Arc/Core/Timestep.h"

#include "Arc/Events/ApplicationEvent.h"
#include "Arc/Events/MouseEvent.h"

namespace ArcEngine
{
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		void OnResize(float width, float height);

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }

		float GetZoomLevel() const { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; }
		void SetZoomSpeed(const float value) { m_ZoomSpeed = value; }
		void SetTranslationSpeed(const float value) { m_CameraTranslationSpeed = value; }
		void SetRotationSpeed(const float value) { m_CameraRotationSpeed = value; }
	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);
	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		OrthographicCamera m_Camera;

		bool m_Rotation;
		
		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f;

		float m_ZoomSpeed = 1.0f;
		float m_CameraTranslationSpeed = 1.0f;
		float m_CameraRotationSpeed = 1.0f;
		
		float m_ZoomSpeedMultiplier = 0.25f;
		float m_CameraTranslationSpeedMultiplier = 1.0f;
		float m_CameraRotationSpeedMultiplier = 90.0f;
	};
}

