#include <ArcEngine.h>

#include "imgui/imgui.h"

class ExampleLayer : public ArcEngine::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f)
	{
		m_VertexArray.reset(ArcEngine::VertexArray::Create());
		
		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};
		std::shared_ptr<ArcEngine::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(ArcEngine::VertexBuffer::Create(vertices, sizeof(vertices)));

		ArcEngine::BufferLayout layout = {
			{ArcEngine::ShaderDataType::Float3, "a_Position" },
			{ArcEngine::ShaderDataType::Float4, "a_Color" }
		};

		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);
		
		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<ArcEngine::IndexBuffer> indexBuffer;
		indexBuffer.reset(ArcEngine::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_SquareVA.reset(ArcEngine::VertexArray::Create());
		float squareVertices[3 * 4] = {
			-0.75f, -0.75f, 0.0f,
 			 0.75f, -0.75f, 0.0f,
			 0.75f,  0.75f, 0.0f,
			-0.75f,  0.75f, 0.0f,
		};
		std::shared_ptr<ArcEngine::VertexBuffer> squareVB;
		squareVB.reset(ArcEngine::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ArcEngine::ShaderDataType::Float3, "a_Position" }
		});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<ArcEngine::IndexBuffer> squareIB;
		squareIB.reset(ArcEngine::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);
		
		std::string vertexSource = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
		
			out vec3 v_Position;
			out vec4 v_Color;
		
			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
			}
		
		)";

		std::string fragmentSource = R"(
			#version 330 core

			layout(location = 0) out vec4 color;
		
			in vec3 v_Position;
			in vec4 v_Color;
		
			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;
			}
		
		)";

		m_Shader.reset(new ArcEngine::Shader(vertexSource, fragmentSource));
		
		// BlueShader
		std::string blueShaderVertexSource = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;

			void main()
			{
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
			}
		
		)";

		std::string blueShaderFragmentSource = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			void main()
			{
				color = vec4(0.2, 0.3, 0.8, 1.0);
			}
		
		)";
		
		m_BlueShader.reset(new ArcEngine::Shader(blueShaderVertexSource, blueShaderFragmentSource));
	}

	virtual void OnUpdate() override
	{
		if(ArcEngine::Input::IsKeyPressed(ARC_KEY_RIGHT))
			m_CameraPosition.x += m_CameraMoveSpeed;
		else if(ArcEngine::Input::IsKeyPressed(ARC_KEY_LEFT))
			m_CameraPosition.x -= m_CameraMoveSpeed;

		if(ArcEngine::Input::IsKeyPressed(ARC_KEY_UP))
			m_CameraPosition.y += m_CameraMoveSpeed;
		else if(ArcEngine::Input::IsKeyPressed(ARC_KEY_DOWN))
			m_CameraPosition.y -= m_CameraMoveSpeed;

		if(ArcEngine::Input::IsKeyPressed(ARC_KEY_A))
			m_CameraRotation += m_CameraRotationSpeed;
		else if(ArcEngine::Input::IsKeyPressed(ARC_KEY_D))
			m_CameraRotation -= m_CameraRotationSpeed;
		
		ArcEngine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		ArcEngine::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		ArcEngine::Renderer::BeginScene(m_Camera);

		ArcEngine::Renderer::Submit(m_BlueShader, m_SquareVA);
		ArcEngine::Renderer::Submit(m_Shader, m_VertexArray);

		ArcEngine::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override
	{
		
	}
	
	virtual void OnEvent(ArcEngine::Event& event) override
	{
		
	}
private:
	std::shared_ptr<ArcEngine::Shader> m_Shader;
	std::shared_ptr<ArcEngine::VertexArray> m_VertexArray;

	std::shared_ptr<ArcEngine::Shader> m_BlueShader;
	std::shared_ptr<ArcEngine::VertexArray> m_SquareVA;

	ArcEngine::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraMoveSpeed = 0.1f;
	
	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 2.0f;
};

class Sandbox : public ArcEngine::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{
		
	}
};

ArcEngine::Application* ArcEngine::CreateApplication()
{
	return new Sandbox();
}
