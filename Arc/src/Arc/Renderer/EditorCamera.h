#pragma once

#include "Arc/Renderer/Camera.h"

#include "Arc/Core/Timestep.h"

namespace ArcEngine
{
	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);
		
		void OnUpdate([[maybe_unused]] Timestep timestep);

		void SetViewportSize(float width, float height);
		void SetPosition(const glm::vec3& position) { m_Position = position; }
		void SetYaw(float yaw) { m_Yaw = yaw; }														// In radians
		void SetPitch(float pitch) { m_Pitch = pitch; }												// In radians

		const glm::mat4& GetView() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }

		const glm::vec3& GetPosition() const { return m_Position; }
		const glm::vec3& GetForward() const { return m_Forward; }
		const glm::vec3& GetRight() const { return m_Right; }
		const glm::vec3& GetUp() const { return m_Up; }
		float GetYaw() const { return m_Yaw; }														// In radians
		float GetPitch() const { return m_Pitch; }													// In radians

	private:
		float m_Fov = glm::radians(45.0f);															// In radians
		float m_AspectRatio = 1.777f;
		float m_NearClip = 0.03f;
		float m_FarClip = 2000.0f;

		glm::vec3 m_Position = glm::vec3(0, 0, -10);
		glm::vec3 m_Forward = glm::vec3(0, 0, 1);
		glm::vec3 m_Right = glm::vec3(1, 0, 0);
		glm::vec3 m_Up = glm::vec3(0, 1, 0);

		float m_Yaw = glm::radians(90.0f);															// In radians
		float m_Pitch = glm::radians(0.0f);															// In radians

		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
	};
}
