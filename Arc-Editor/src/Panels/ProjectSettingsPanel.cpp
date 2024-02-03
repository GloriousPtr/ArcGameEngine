#include "ProjectSettingsPanel.h"

#include <Icons.h>

#include "../Utils/UI.h"

namespace ArcEngine
{
	ProjectSettingsPanel::ProjectSettingsPanel()
		: BasePanel("Project Settings", ARC_ICON_SETTINGS)
	{
	}

	void ProjectSettingsPanel::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();

		if (OnBegin())
		{
			auto& layerCollisionMask = Scene::LayerCollisionMask;

			if (ImGui::TreeNode("Layers"))
			{
				if (layerCollisionMask.size() < 16)
				{
					const float cursorPosY = ImGui::GetCursorPosY();
					if (UI::IconButton("  " ARC_ICON_PLUS, "Add  "))
						layerCollisionMask[BIT(layerCollisionMask.size())] = { "Layer", 0xFFFF, static_cast<uint8_t>(layerCollisionMask.size()) };

					ImGui::SetCursorPosY(cursorPosY + UI::GetIconButtonSize("  " ARC_ICON_PLUS, "Add  ").y + ImGui::GetStyle().FramePadding.y);
				}

				int i = 0;
				EntityLayer deletedLayer = 0;
				for (auto& [layer, layerData] : layerCollisionMask)
				{
					ImGui::PushID(i);

					const bool disabled = i <= 1;
					if (disabled)
						ImGui::BeginDisabled();

					constexpr size_t bufferSize = 256;
					char buffer[bufferSize];
					memcpy_s(buffer, bufferSize, layerData.Name.c_str(), layerData.Name.size() + 1);
					if (ImGui::InputText("##LayerName", buffer, bufferSize))
						layerData.Name = buffer;

					if (disabled)
					{
						ImGui::EndDisabled();
						if (i == 0)
						{
							ImGui::SameLine();
							ImGui::TextUnformatted(ARC_ICON_INFO);
							if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
							{
								ImGui::BeginTooltip();
								ImGui::TextUnformatted("Entities marked with Static layer will not collide with each other.");
								ImGui::EndTooltip();
							}
						}
					}
					else
					{
						ImGui::SameLine();
						if (ImGui::Button(ARC_ICON_CLOSE))
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
				const int colCount = static_cast<int>(layerCollisionMask.size());
				if (ImGui::BeginTable("table1", colCount + 1, ImGuiTableFlags_SizingFixedFit))
				{
					ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
					for (int c = colCount - 1; c >= 0; --c)
					{
						EntityLayer colLayer = BIT(c);
						const char* columnLayerName = layerCollisionMask.at(colLayer).Name.c_str();
						ImGui::TableSetupColumn(columnLayerName, ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed);
					}
					ImGui::TableAngledHeadersRow();
					int i = 0;
					for (int row = 0; row < colCount; ++row)
					{
						ImGui::TableNextRow();
						for (int column = 0; column < colCount + 1; ++column)
						{
							ImGui::TableSetColumnIndex(column);

							if (column == 0)
							{
								EntityLayer rowLayer = BIT(row);
								const char* rowLayerName = layerCollisionMask.at(rowLayer).Name.c_str();

								ImGui::TextUnformatted(rowLayerName);
								continue;
							}

							const int r = row;
							const int c = colCount - column;
							if (c < r)
								continue;

							EntityLayer rowLayer = BIT(r);
							EntityLayer colLayer = BIT(c);
							const char* rowLayerName = layerCollisionMask.at(rowLayer).Name.c_str();
							const char* columnLayerName = layerCollisionMask.at(colLayer).Name.c_str();

							bool on = (rowLayer & layerCollisionMask.at(colLayer).Flags) == rowLayer;
							on = on && ((colLayer & layerCollisionMask.at(rowLayer).Flags) == colLayer);

							ImGui::PushID(i);
							++i;

							const bool disabled = rowLayer == 1 && colLayer == 1;
							if (disabled)
							{
								if (on)
								{
									layerCollisionMask.at(colLayer).Flags &= ~rowLayer;
									layerCollisionMask.at(rowLayer).Flags &= ~colLayer;
								}
								ImGui::BeginDisabled();
							}

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

							if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay | ImGuiHoveredFlags_AllowWhenDisabled))
							{
								ImGui::BeginTooltip();
								ImGui::Text("%s / %s", rowLayerName, columnLayerName);
								ImGui::EndTooltip();
							}

							if (disabled)
								ImGui::EndDisabled();
							
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
