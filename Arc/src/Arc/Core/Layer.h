#pragma once

#include "Arc/Core/Timestep.h"
#include "Arc/Events/Event.h"

namespace ArcEngine
{
	struct WorkQueue;

	class Layer
	{
	public:
		explicit Layer(const eastl::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() { /* Called when layer is attached */ }
		virtual void OnDetach() { /* Called when layer is detached */ }
		virtual void OnUpdate(Timestep ts, WorkQueue* queue) { /* Layer OnUpdate */ }
		virtual void OnImGuiRender(WorkQueue* queue) { /* Layer OnRender */ }
		virtual void OnEvent(Event& e) { /* Called when an event is fired */ }

		[[nodiscard]] const eastl::string& GetName() const { return m_DebugName; }
	protected:
		eastl::string m_DebugName;
	};
}
