#pragma once
#include <ArcEngine.h>

namespace ArcEngine
{
	class RendererSettingsPanel
	{
	public:
		RendererSettingsPanel();
		virtual ~RendererSettingsPanel();

		void OnImGuiRender();
	};
}
