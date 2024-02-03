#include "StatsPanel.h"

#include <Icons.h>

#include "../Utils/UI.h"
#include <Arc/Renderer/GraphicsContext.h>

namespace ArcEngine
{
	StatsPanel::StatsPanel(const char* name)
		: BasePanel(name, ARC_ICON_INFO_2, true), m_FpsValues{}
	{
	}

	void StatsPanel::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();
		
		float avg = 0.0;

		const size_t size = m_FrameTimes.size();
		if (size >= 50)
			m_FrameTimes.erase(m_FrameTimes.begin());

		m_FrameTimes.emplace_back(ImGui::GetIO().Framerate);
		for (uint32_t i = 0; i < size; i++)
		{
			const float frameTime = m_FrameTimes[i];
			m_FpsValues[i] = frameTime;
			avg += frameTime;
		}
		
		avg /= static_cast<float>(size);

		if (OnBegin())
		{
			{
				const auto stats = Renderer2D::GetStats();
				ImGui::Text("2D");

				ImGui::Text("Draw Calls: %d", stats.DrawCalls);
				ImGui::SameLine();
				ImGui::PushItemWidth(-1);
				ImGui::Text("Quads: %d", stats.QuadCount);
				ImGui::SameLine();
				ImGui::PushItemWidth(-1);
				ImGui::Text("Tris: %d", stats.GetTotalTriangleCount());

				ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
				ImGui::SameLine();
				ImGui::PushItemWidth(-1);
				ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
			}

			ImGui::Separator();

			{
				const auto stats = Renderer3D::GetStats();
				ImGui::Text("3D");

				ImGui::Text("Draw Calls: %d", stats.DrawCalls);
				ImGui::SameLine();
				ImGui::PushItemWidth(-1);
				ImGui::Text("Indices: %d", stats.IndexCount);
			}

			Window& window = Application::Get().GetWindow();

			ImGui::Separator();
			{
				GraphicsContext::Stats gpuStats;
				window.GetGraphicsContext()->GetStats(gpuStats);

				ImGui::TextUnformatted("GPU");
				ImGui::Text("Heaps: %u (%.2f MB)", gpuStats.BlockCount, static_cast<double>(gpuStats.BlockBytes) / (1024.0 * 1024.0));
				ImGui::Text("Allocations: %u (%.2f MB)", gpuStats.AllocationCount, static_cast<double>(gpuStats.AllocationBytes) / (1024.0 * 1024.0));
				ImGui::Text("Usage : %.2f/%.2f MB", static_cast<double>(gpuStats.UsageBytes) / (1024.0 * 1024.0), static_cast<double>(gpuStats.BudgetBytes) / (1024.0 * 1024.0));
			}

			UI::BeginProperties();

			bool vSync = window.IsVSync();
			if (UI::Property("VSync Enabled", vSync))
				window.SetVSync(vSync);
			UI::EndProperties();

			ImGui::PlotLines("##FPS", m_FpsValues, static_cast<int>(size));
			ImGui::Text("FPS: %lf", static_cast<double>(avg));
			const double fps = (1.0 / static_cast<double>(avg)) * 1000.0;
			ImGui::Text("Frame time (ms): %lf", fps);

			OnEnd();
		}
	}
}
