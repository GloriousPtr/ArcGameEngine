#pragma once

#include "Arc/Core/Layer.h"

namespace ArcEngine
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent([[maybe_unused]] Event& e) override;

		void Begin() const;
		void End() const;

		void SetBlockEvents(bool block) { m_BlockEvents = block; }
	private:
		bool m_BlockEvents = true;
	};
}
