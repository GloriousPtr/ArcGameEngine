#include "ProjectSettingsPanel.h"

#include <ArcEngine.h>
#include <icons/IconsMaterialDesignIcons.h>
#include <imgui/imgui.h>

#include "../Utils/UI.h"

namespace ArcEngine
{
	ProjectSettingsPanel::ProjectSettingsPanel()
		: BasePanel("Project Settings", ICON_MDI_SETTINGS)
	{
	}

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
						EntityLayerData newLayer = { "Layer", 0xFFFF, (uint8_t)layerCollisionMask.size() };
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

					const bool disabled = i <= 1;
					if (disabled)
						ImGui::BeginDisabled();

					if (ImGui::InputText("##LayerName", buffer, size))
						layerData.Name = buffer;

					if (disabled)
					{
						ImGui::EndDisabled();
						if (i == 0)
						{
							ImGui::SameLine();
							ImGui::Text(ICON_MDI_INFORMATION);
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
				const int colCount = (int)layerCollisionMask.size() + 1;
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
								UI::AddTextVertical(drawList, columnLayerName, ImGui::GetCursorScreenPos(), ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 1.0f }));
								continue;
							}

							if (c < r)
								continue;

							bool on = (rowLayer & layerCollisionMask.at(colLayer).Flags) == rowLayer;
							on = on && ((colLayer & layerCollisionMask.at(rowLayer).Flags) == colLayer);

							ImGui::PushID(i);
							++i;

							const bool disabled = c == 0 && r == 0;
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
