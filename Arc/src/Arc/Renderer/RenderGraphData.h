#pragma once

#include "Framebuffer.h"

namespace ArcEngine
{
	struct RenderGraphData
	{
		Ref<Framebuffer> CompositePassTarget;
		Ref<Framebuffer> RenderPassTarget;

		static const size_t BlurSamples = 6;
		Ref<Framebuffer> PrefilteredFramebuffer;
		Ref<Framebuffer> TempBlurFramebuffers[BlurSamples];
		Ref<Framebuffer> DownsampledFramebuffers[BlurSamples];
		Ref<Framebuffer> UpsampledFramebuffers[BlurSamples];

		void Resize(uint32_t width,  uint32_t height)
		{
			CompositePassTarget->Resize(width, height);
			RenderPassTarget->Resize(width, height);

			width /= 2;
			height /= 2;
			PrefilteredFramebuffer->Resize(width, height);

			for (size_t i = 0; i < BlurSamples; i++)
			{
				width /= 2;
				height /= 2;
				TempBlurFramebuffers[i]->Resize(width, height);
				DownsampledFramebuffers[i]->Resize(width, height);
				UpsampledFramebuffers[i]->Resize(width, height);
			}
		}
	};
}
