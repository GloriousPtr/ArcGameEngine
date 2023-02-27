#pragma once

#include "Arc/Renderer/GraphicsContext.h"

namespace ArcEngine
{
	class Dx12Context : public GraphicsContext
	{
	public:
		explicit Dx12Context(HWND hwnd);

		void Init() override;
		void SwapBuffers() override;
	private:
		HWND m_Hwnd;
	};
}
