#pragma once

#include "ArcEngine.h"

class Sandbox2D : public ArcEngine::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;
	
	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(ArcEngine::Timestep ts) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(ArcEngine::Event& e) override;
private:
	ArcEngine::OrthographicCameraController m_CameraController;

	// Temp
	ArcEngine::Ref<ArcEngine::VertexArray> m_SquareVA;
	ArcEngine::Ref<ArcEngine::Shader> m_FlatColorShader;
	ArcEngine::Ref<ArcEngine::Framebuffer> m_Framebuffer;

	ArcEngine::Ref<ArcEngine::Texture2D> m_CheckerboardTexture;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };

	float frameTime = 0.0f;
};
