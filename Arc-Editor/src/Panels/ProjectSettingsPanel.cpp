#include "ProjectSettingsPanel.h"

#include <ArcEngine.h>
#include <imgui/imgui.h>

#include "../Utils/UI.h"

namespace ArcEngine
{
	void ProjectSettingsPanel::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();

		if (OnBegin())
		{
			static const char* btnTitle = "Reload Assemblies";
			ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(btnTitle).x - ImGui::GetStyle().WindowPadding.x);

			if (ImGui::Button(btnTitle))
				ScriptEngine::ReloadAppDomain();

			ImGui::Separator();

			auto& layerCollisionMask = Scene::LayerCollisionMask;

			if (ImGui::TreeNode("Layers"))
			{
				if (layerCollisionMask.size() < 16)
				{
					float cursorPosY = ImGui::GetCursorPosY();
					if (UI::IconButton("  " ICON_MDI_PLUS, "Add  "))
					{
						EntityLayerData newLayer = { "Layer", 0xFFFFFF };
						layerCollisionMask[BIT(layerCollisionMask.size())] = newLayer;
					}

					ImGui::SetCursorPosY(cursorPosY + UI::GetIconButtonSize("  " ICON_MDI_PLUS, "Add  ").y + ImGui::GetStyle().FramePadding.y);
				}

				int i = 0;
				EntityLayer deletedLayer = 0;
				for (auto& [layer, layerData] : layerCollisionMask)
				{
					constexpr size_t size = 256;
					char buffer[size];
					memcpy(buffer, layerData.Name.data(), size);

					ImGui::PushID(i);
					if (ImGui::InputText("##LayerName", buffer, size))
						layerData.Name = buffer;

					if (i != 0)
					{
						float buttonSize = ImGui::GetFrameHeight();
						ImGui::SameLine();
						if (ImGui::Button(ICON_MDI_CLOSE))
							deletedLayer = layer;
					}
					ImGui::PopID();
					++i;
				}

				if (deletedLayer != 0)
					layerCollisionMask.erase(deletedLayer);

				ImGui::TreePop();
			}

			ImGui::Separator();

			if (ImGui::TreeNode("Collision Matrix"))
			{
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);
				int colCount = (int)layerCollisionMask.size() + 1;
				if (ImGui::BeginTable("table1", colCount, ImGuiTableFlags_SizingFixedFit))
				{
					int i = 0;
					for (int row = 0; row < colCount; ++row)
					{
						ImGui::TableNextRow();
						for (int column = 0; column < colCount; ++column)
						{
							ImGui::TableSetColumnIndex(column);

							if (row == 0 && column == 0)
								continue;

							int r = row - 1;
							EntityLayer rowLayer = BIT(r);
							const char* rowLayerName = layerCollisionMask.at(rowLayer).Name.c_str();
							if (column == 0 && row != 0)
							{
								ImGui::TextUnformatted(rowLayerName);
								continue;
							}

							int c = colCount - 1 - column;
							EntityLayer colLayer = BIT(c);
							const char* columnLayerName = layerCollisionMask.at(colLayer).Name.c_str();
							if (row == 0 && column != 0)
							{
								auto* drawList = ImGui::GetWindowDrawList();
								ImVec2 r = ImGui::GetCursorScreenPos();
								UI::AddTextVertical(drawList, columnLayerName, { r.x, r.y }, ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 1.0f }));
								continue;
							}

							if (c < r)
								continue;

							bool on = (rowLayer & layerCollisionMask.at(colLayer).Flags) == rowLayer;
							on = on && ((colLayer & layerCollisionMask.at(rowLayer).Flags) == colLayer);

							ImGui::PushID(i++);
							if (ImGui::Checkbox("##mat", &on))
							{
								if (on)
								{
									layerCollisionMask.at(colLayer).Flags |= rowLayer;
									layerCollisionMask.at(rowLayer).Flags |= colLayer;
								}
								else
								{
									layerCollisionMask.at(colLayer).Flags &= ~rowLayer;
									layerCollisionMask.at(rowLayer).Flags &= ~colLayer;
								}
							}
							if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
							{
								ImGui::BeginTooltip();
								ImGui::Text("%s / %s", rowLayerName, columnLayerName);
								ImGui::EndTooltip();
							}
							ImGui::PopID();
						}
					}
					ImGui::EndTable();
				}

				ImGui::TreePop();
			}

			OnEnd();
		}
	}
}
