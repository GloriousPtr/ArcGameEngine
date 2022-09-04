#pragma once

#include "Framebuffer.h"
#include "Arc/Debug/Profiler.h"

namespace ArcEngine
{
	struct RenderGraphData
	{
		Ref<Framebuffer> CompositePassTarget;
		Ref<Framebuffer> RenderPassTarget;
		Ref<Framebuffer> LightingPassTarget;

		static const size_t MaxBlurSamples = 6;
		size_t BlurSamples = MaxBlurSamples;
		Ref<Framebuffer> PrefilteredFramebuffer;
		Ref<Framebuffer> TempBlurFramebuffers[MaxBlurSamples];
		Ref<Framebuffer> DownsampledFramebuffers[MaxBlurSamples];
		Ref<Framebuffer> UpsampledFramebuffers[MaxBlurSamples];

		void Resize(uint32_t width, uint32_t height)
		{
			ARC_PROFILE_SCOPE();

			CompositePassTarget->Resize(width, height);
			RenderPassTarget->Resize(width, height);
			LightingPassTarget->Resize(width, height);

			width /= 2;
			height /= 2;
			PrefilteredFramebuffer->Resize(width, height);

			BlurSamples = 0;
			for (size_t i = 0; i < MaxBlurSamples; i++)
			{
				++BlurSamples;

				width /= 2;
				height /= 2;

				if (width <= 0 || height <= 0)
					break;

				TempBlurFramebuffers[i]->Resize(width, height);
				DownsampledFramebuffers[i]->Resize(width, height);
				UpsampledFramebuffers[i]->Resize(width, height);
			}
		}
	};
}
