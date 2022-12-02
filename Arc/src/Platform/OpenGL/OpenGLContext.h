#pragma once
#include "Arc/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace ArcEngine
{
	class OpenGLContext: public GraphicsContext
	{
	public:
		explicit OpenGLContext(GLFWwindow* windowHandle);

		void Init() override;
		void SwapBuffers() override;
	private:
		GLFWwindow* m_WindowHandle;
	};
}
