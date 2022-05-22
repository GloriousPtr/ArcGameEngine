#pragma once

#include <EASTL/vector.h>

#include "Arc/Core/Base.h"
#include "Arc/Core/Layer.h"

namespace ArcEngine
{
	class LayerStack
	{
	public:
		LayerStack() = default;
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		eastl::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		eastl::vector<Layer*>::iterator end() { return m_Layers.end(); }
		eastl::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
		eastl::vector<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }

		eastl::vector<Layer*>::const_iterator begin() const { return m_Layers.begin(); }
		eastl::vector<Layer*>::const_iterator end()	const { return m_Layers.end(); }
		eastl::vector<Layer*>::const_reverse_iterator rbegin() const { return m_Layers.rbegin(); }
		eastl::vector<Layer*>::const_reverse_iterator rend() const { return m_Layers.rend(); }
	private:
		eastl::vector<Layer*> m_Layers;
		unsigned int m_LayerInsertIndex = 0;
	};
}

