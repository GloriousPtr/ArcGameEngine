#include "arcpch.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace ArcEngine
{
	void OpenGLMessageCallback(
		[[maybe_unused]] unsigned source,
		[[maybe_unused]] unsigned type,
		[[maybe_unused]] unsigned id,
		[[maybe_unused]] unsigned severity,
		[[maybe_unused]] int length,
		[[maybe_unused]] const char* message,
		[[maybe_unused]] const void* userParam)
	{
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:			ARC_CORE_CRITICAL(message); return;
			case GL_DEBUG_SEVERITY_MEDIUM:			ARC_CORE_ERROR(message); return;
			case GL_DEBUG_SEVERITY_LOW:				ARC_CORE_WARN(message); return;
			case GL_DEBUG_SEVERITY_NOTIFICATION:	ARC_CORE_TRACE(message); return;
			default:								ARC_CORE_ERROR(message); return;
		}

		ARC_CORE_ASSERT(false, "Unknown severity level!");
	}
	
	void OpenGLRendererAPI::Init()
	{
		ARC_PROFILE_SCOPE();

		#ifdef ARC_DEBUG
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(OpenGLMessageCallback, nullptr);

			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
		#endif
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		ARC_PROFILE_SCOPE();

		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		ARC_PROFILE_SCOPE();

		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		ARC_PROFILE_SCOPE();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		ARC_PROFILE_SCOPE();

		vertexArray->Bind();
		const uint32_t count = indexCount != 0 ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRendererAPI::Draw(const Ref<VertexArray>& vertexArray, uint32_t count)
	{
		ARC_PROFILE_SCOPE();

		vertexArray->Bind();
		glDrawArrays(GL_TRIANGLES, 0, count);
	}

	void OpenGLRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		ARC_PROFILE_SCOPE();

		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	void OpenGLRendererAPI::EnableCulling()
	{
		ARC_PROFILE_SCOPE();

		glEnable(GL_CULL_FACE);
	}

	void OpenGLRendererAPI::DisableCulling()
	{
		ARC_PROFILE_SCOPE();

		glDisable(GL_CULL_FACE);
	}

	void OpenGLRendererAPI::FrontCull()
	{
		ARC_PROFILE_SCOPE();

		glCullFace(GL_FRONT);
	}

	void OpenGLRendererAPI::BackCull()
	{
		ARC_PROFILE_SCOPE();

		glCullFace(GL_BACK);
	}

	void OpenGLRendererAPI::SetDepthMask(bool value)
	{
		ARC_PROFILE_SCOPE();

		glDepthMask(value);
	}

	void OpenGLRendererAPI::SetDepthTest(bool value)
	{
		ARC_PROFILE_SCOPE();

		if (value)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	void OpenGLRendererAPI::SetBlendState(bool value)
	{
		if (value)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}
}
