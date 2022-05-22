#pragma once

#include "Arc/Core/Base.h"
#include "Arc/Core/Timestep.h"
#include "Arc/Events/Event.h"

namespace ArcEngine
{
	class Layer
	{
	public:
		Layer(const eastl::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& e) {}

		inline const eastl::string& GetName() const { return m_DebugName; }
	protected:
		eastl::string m_DebugName;
	};
}

