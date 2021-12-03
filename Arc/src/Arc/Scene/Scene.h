#pragma once

#include "Arc/Core/Timestep.h"
#include "Arc/Renderer/EditorCamera.h"

#include "entt.hpp"

namespace ArcEngine
{
	class Entity;
	
	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);
		bool HasEntity(uint32_t entity);

		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnUpdateRuntime(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);
		void MarkViewportDirty() { m_ViewportDirty = true; }

		bool IsViewportDirty() { return m_ViewportDirty; }
		int GetPixelDataAtPoint(const int x, const int y);
		Entity GetPrimaryCameraEntity();
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		entt::registry m_Registry;
		std::unordered_map<uint32_t, entt::entity> m_EntityMap;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		bool m_ViewportDirty = true;
		
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}
