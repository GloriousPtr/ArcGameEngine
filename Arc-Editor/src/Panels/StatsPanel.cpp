#include "StatsPanel.h"

#include <imgui/imgui.h>

#include "../Utils/UI.h"

namespace ArcEngine
{
	void StatsPanel::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();
		
		float avg = 0.0f;

		const uint32_t size = m_FrameTimes.size();
		if (size >= 50)
			m_FrameTimes.erase(m_FrameTimes.begin());

		m_FrameTimes.push_back(ImGui::GetIO().Framerate);
		for (uint32_t i = 0; i < size; i++)
		{
			m_FpsValues[i] = m_FrameTimes[i];
			avg += m_FrameTimes[i];
		}
		
		avg /= size;

		ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiCond_FirstUseEver);
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


			UI::BeginProperties();
			bool vSync = Application::Get().GetWindow().IsVSync();
			if (UI::Property("VSync Enabled", vSync))
				Application::Get().GetWindow().SetVSync(vSync);
			UI::EndProperties();

			ImGui::PlotLines("##FPS", m_FpsValues, size);
			ImGui::Text("FPS: %f", avg);
			const float fps = (1.0f / avg) * 1000.0f;
			ImGui::Text("Frame time (ms): %f", fps);

			OnEnd();
		}
	}
}
