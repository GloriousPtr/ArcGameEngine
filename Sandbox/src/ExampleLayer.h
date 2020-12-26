#pragma once

#include "ArcEngine.h"

class ExampleLayer : public ArcEngine::Layer
{
public:
	ExampleLayer();
	virtual ~ExampleLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(ArcEngine::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(ArcEngine::Event& e) override;
private:
	ArcEngine::ShaderLibrary m_ShaderLibrary;
	ArcEngine::Ref<ArcEngine::Shader> m_Shader;
	ArcEngine::Ref<ArcEngine::VertexArray> m_VertexArray;

	ArcEngine::Ref<ArcEngine::Shader> m_FlatColorShader;
	ArcEngine::Ref<ArcEngine::VertexArray> m_SquareVA;

	ArcEngine::Ref<ArcEngine::Texture2D> m_Texture, m_ChernoLogoTexture;

	ArcEngine::OrthographicCameraController m_CameraController;
	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};

