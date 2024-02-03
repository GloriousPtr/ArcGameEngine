#pragma once

namespace ArcEngine
{
	using WindowHandle = void*;
	using GraphicsCommandList = void*;

	class GraphicsContext
	{
	public:
		struct Stats
		{
			uint32_t BlockCount = 0;
			uint32_t AllocationCount = 0;
			uint64_t BlockBytes = 0;
			uint64_t AllocationBytes = 0;
			uint64_t UsageBytes = 0;
			uint64_t BudgetBytes = 0;
		};

		virtual ~GraphicsContext() = default;
		
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
		virtual void SetSyncInterval(uint32_t value) = 0;
		virtual void GetStats(Stats& stats) = 0;

		[[nodiscard]] static Scope<GraphicsContext> Create(WindowHandle window);
	};
}
