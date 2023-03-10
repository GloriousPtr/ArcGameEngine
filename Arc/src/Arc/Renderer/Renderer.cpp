#include "arcpch.h"
#include "Arc/Renderer/Renderer.h"
#include "Arc/Renderer/Renderer2D.h"
#include "Arc/Renderer/Renderer3D.h"

#include "Shader.h"
#include "Buffer.h"

namespace ArcEngine
{
	Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;

	void Renderer::Init()
	{
		ARC_PROFILE_SCOPE()
		
		RenderCommand::Init();

		auto s = Shader::Create("assets/shaders/TestShader.hlsl",
		{
			{ ShaderDataType::Float3, "POSITION"  },
			{ ShaderDataType::Float3, "NORMAL"    },
			{ ShaderDataType::Float3, "TANGENT"   },
			{ ShaderDataType::Float3, "BINORMAL" },
			{ ShaderDataType::Float2, "TEXCOORD"  }
		});

		//Renderer2D::Init();
		//Renderer3D::Init();
	}

	void Renderer::Shutdown()
	{
		ARC_PROFILE_SCOPE()

		//Renderer2D::Shutdown();
		//Renderer3D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		ARC_PROFILE_SCOPE()

		RenderCommand::SetViewport(0, 0, width, height);
	}
}
