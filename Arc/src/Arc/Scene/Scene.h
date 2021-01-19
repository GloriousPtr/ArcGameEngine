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

		void CopyTo(Ref<Scene>& scene);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithID(const uint32_t id);
		void DestroyEntity(Entity entity);

		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnRuntimeStart();
		void OnRuntimeEnd();
		void OnUpdateRuntime(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);

		int GetPixelDataAtPoint(const int x, const int y);
		
		Entity GetPrimaryCameraEntity();
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		
		std::unordered_map<uint32_t, entt::entity> m_EntityMap;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}
