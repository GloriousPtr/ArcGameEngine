#include "arcpch.h"
#include "Platform/OpenGL/OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace ArcEngine
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		ARC_CORE_ASSERT(m_WindowHandle, "Handle is null!");
	}

	void OpenGLContext::Init()
	{
		ARC_PROFILE_SCOPE();
		
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		ARC_CORE_ASSERT(status, "Failed to initialize Glad!");

		ARC_CORE_INFO("OpenGL Info:");
		ARC_CORE_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
		ARC_CORE_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
		ARC_CORE_INFO("  Version: {0}", glGetString(GL_VERSION));

		ARC_CORE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "Arc Engine requires at least OpenGL version 4.5!");
	}

	void OpenGLContext::SwapBuffers()
	{
		ARC_PROFILE_SCOPE();
		
		glfwSwapBuffers(m_WindowHandle);
	}
}

