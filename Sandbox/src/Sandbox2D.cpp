#include "Sandbox2D.h"

#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui.h"

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{
}

void Sandbox2D::OnAttach()
{
	m_CheckerboardTexture = ArcEngine::Texture2D::Create("assets/textures/Checkerboard.png");
}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnUpdate(ArcEngine::Timestep ts)
{
	// Update
	m_CameraController.OnUpdate(ts);

	//Render
	ArcEngine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	ArcEngine::RenderCommand::Clear();

	ArcEngine::Renderer2D::BeginScene(m_CameraController.GetCamera());
	ArcEngine::Renderer2D::DrawQuad({ -1.0f, 0.0f }, 45.0f, { 0.8f, 0.8f }, nullptr,	{ 0.8f, 0.2f, 0.3f, 1.0f });
	ArcEngine::Renderer2D::DrawQuad({ 0.5f, -0.5f }, -60.0f, { 0.5f, 0.75f }, nullptr, { 0.2f, 0.3f, 0.8f, 1.0f });
	ArcEngine::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, 0.0f, { 10.0f, 10.0f }, m_CheckerboardTexture);
	ArcEngine::Renderer2D::EndScene();

	// std::dynamic_pointer_cast<ArcEngine::OpenGLShader>(m_FlatColorShader)->Bind();
	// std::dynamic_pointer_cast<ArcEngine::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat4("u_Color", m_SquareColor);
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(ArcEngine::Event& e)
{
	m_CameraController.OnEvent(e);
}
