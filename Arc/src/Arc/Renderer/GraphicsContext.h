#pragma once

namespace ArcEngine
{
	using WindowHandle = void*;
	using GraphicsCommandList = void*;

	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;
		
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
		virtual void SetSyncInterval(uint32_t value) = 0;

		[[nodiscard]] static Scope<GraphicsContext> Create(WindowHandle window);
	};
}
