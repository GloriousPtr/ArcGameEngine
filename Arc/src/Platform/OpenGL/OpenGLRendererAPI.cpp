#include "arcpch.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace ArcEngine
{
	void OpenGLMessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		unsigned severity,
		int length,
		const char* message,
		const void* userParam)
	{
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:         ARC_CORE_CRITICAL(message); return;
			case GL_DEBUG_SEVERITY_MEDIUM:       ARC_CORE_ERROR(message); return;
			case GL_DEBUG_SEVERITY_LOW:          ARC_CORE_WARN(message); return;
			case GL_DEBUG_SEVERITY_NOTIFICATION: ARC_CORE_TRACE(message); return;
		}

		ARC_CORE_ASSERT(false, "Unknown severity level!");
	}
	
	void OpenGLRendererAPI::Init()
	{
		OPTICK_EVENT();

		ARC_PROFILE_FUNCTION();

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
		OPTICK_EVENT();

		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		OPTICK_EVENT();

		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		OPTICK_EVENT();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		OPTICK_EVENT();

		vertexArray->Bind();
		const uint32_t count = indexCount != 0 ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRendererAPI::Draw(const Ref<VertexArray>& vertexArray, uint32_t count)
	{
		OPTICK_EVENT();

		vertexArray->Bind();
		glDrawArrays(GL_TRIANGLES, 0, count);
	}

	void OpenGLRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		OPTICK_EVENT();

		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	void OpenGLRendererAPI::EnableCulling()
	{
		OPTICK_EVENT();

		glEnable(GL_CULL_FACE);
	}

	void OpenGLRendererAPI::DisableCulling()
	{
		OPTICK_EVENT();

		glDisable(GL_CULL_FACE);
	}

	void OpenGLRendererAPI::FrontCull()
	{
		OPTICK_EVENT();

		glCullFace(GL_FRONT);
	}

	void OpenGLRendererAPI::BackCull()
	{
		OPTICK_EVENT();

		glCullFace(GL_BACK);
	}

	void OpenGLRendererAPI::SetDepthMask(bool value)
	{
		OPTICK_EVENT();

		glDepthMask(value);
	}

	void OpenGLRendererAPI::SetDepthTest(bool value)
	{
		OPTICK_EVENT();

		if (value)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}
}
