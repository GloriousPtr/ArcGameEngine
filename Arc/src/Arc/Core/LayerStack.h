#pragma once

#include "Arc/Core/Layer.h"

namespace ArcEngine
{
	class LayerStack
	{
	public:
		LayerStack() = default;
		~LayerStack();

		LayerStack(const LayerStack& other) = delete;
		LayerStack(LayerStack&& other) = delete;

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		[[nodiscard]] eastl::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		[[nodiscard]] eastl::vector<Layer*>::iterator end() { return m_Layers.end(); }
		[[nodiscard]] eastl::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
		[[nodiscard]] eastl::vector<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }

		[[nodiscard]] eastl::vector<Layer*>::const_iterator begin() const { return m_Layers.begin(); }
		[[nodiscard]] eastl::vector<Layer*>::const_iterator end()	const { return m_Layers.end(); }
		[[nodiscard]] eastl::vector<Layer*>::const_reverse_iterator rbegin() const { return m_Layers.rbegin(); }
		[[nodiscard]] eastl::vector<Layer*>::const_reverse_iterator rend() const { return m_Layers.rend(); }

	private:
		eastl::vector<Layer*> m_Layers;
		unsigned int m_LayerInsertIndex = 0;
	};
}

