#include "Sandbox2D.h"

#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui.h"

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{
}

void Sandbox2D::OnAttach()
{
	ARC_PROFILE_FUNCTION();
	
	m_CheckerboardTexture = ArcEngine::Texture2D::Create("assets/textures/Checkerboard.png");
}

void Sandbox2D::OnDetach()
{
	ARC_PROFILE_FUNCTION();
	
}

void Sandbox2D::OnUpdate(ArcEngine::Timestep ts)
{
	ARC_PROFILE_FUNCTION();

	frameTime = ts;
	
	// Update
	m_CameraController.OnUpdate(ts);

	//Render
	ArcEngine::Renderer2D::ResetStats();
	ArcEngine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
	ArcEngine::RenderCommand::Clear();

	static float rotation = 0.0f;
	rotation += ts * 50.0f;

	ArcEngine::Renderer2D::BeginScene(m_CameraController.GetCamera());
	ArcEngine::Renderer2D::DrawQuad({ -1.0f, 0.0f }, rotation, { 0.8f, 0.8f }, nullptr,	{ 0.8f, 0.2f, 0.3f, 1.0f });
	ArcEngine::Renderer2D::DrawQuad({ 0.5f, -0.5f }, -rotation, { 0.5f, 0.75f }, nullptr, m_SquareColor);
	ArcEngine::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, 0.0f, { 20.0f, 20.0f }, m_CheckerboardTexture, glm::vec4(1.0f), 10.0f);

	for (float y = -5.0f; y < 5.0f; y += 0.5f)
	{
		for (float x = -5.0f; x < 5.0f; x += 0.5f)
		{
			glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.5f };
			ArcEngine::Renderer2D::DrawQuad({ x, y }, 0.0f, { 0.45f, 0.45f }, nullptr, color);
		}
	}
	ArcEngine::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	ARC_PROFILE_FUNCTION();

	ImGui::Begin("Settings");

	auto stats = ArcEngine::Renderer2D::GetStats();
	ImGui::Text("Renderer2D Stats:");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

	static float frameTimeRefreshTimer = 0.0f;
	static float ft = 0.0f;
	static float frameRate = 0.0f;
	frameTimeRefreshTimer += frameTime;
	if(frameTimeRefreshTimer >= 0.25f)
	{
		ft = frameTime;
		frameRate = 1.0f / frameTime;
		frameTimeRefreshTimer = 0.0f;
	}
	ImGui::Text("FrameTime: %f ms", ft);
	ImGui::Text("FPS: %d", (int)frameRate);
	
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(ArcEngine::Event& e)
{
	m_CameraController.OnEvent(e);
}
