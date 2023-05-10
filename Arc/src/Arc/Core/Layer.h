#pragma once

#include "Arc/Core/Timestep.h"
#include "Arc/Events/Event.h"

namespace ArcEngine
{
	class Layer
	{
	public:
		explicit Layer(const eastl::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() { /* Called when layer is attached */ }
		virtual void OnDetach() { /* Called when layer is detached */ }
		virtual void OnUpdate([[maybe_unused]] Timestep ts) { /* Layer OnUpdate */ }
		virtual void OnImGuiRender() { /* Layer OnRender */ }
		virtual void OnEvent([[maybe_unused]] Event& e) { /* Called when an event is fired */ }

		[[nodiscard]] const eastl::string& GetName() const { return m_DebugName; }
	protected:
		eastl::string m_DebugName;
	};
}
