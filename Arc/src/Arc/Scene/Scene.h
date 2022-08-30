#pragma once

#include "Arc/Core/UUID.h"
#include "Arc/Core/Timestep.h"
#include "Arc/Renderer/EditorCamera.h"
#include "Arc/Renderer/RenderGraphData.h"

#include "entt.hpp"

class b2World;
class b2Fixture;

namespace ArcEngine
{
	class ContactListener;
	class Entity;
	
	class Scene
	{
	public:
		Scene();
		~Scene();

		static Ref<Scene> CopyTo(Ref<Scene> other);

		Entity CreateEntity(const eastl::string& name = eastl::string());
		Entity CreateEntityWithUUID(UUID uuid, const eastl::string& name = eastl::string());
		void DestroyEntity(Entity entity);
		bool HasEntity(UUID uuid);
		Entity GetEntity(UUID uuid);

		void OnUpdateEditor(Timestep ts, Ref<RenderGraphData>& renderGraphData, EditorCamera& camera);
		void OnUpdateRuntime(Timestep ts, Ref<RenderGraphData>& renderGraphData, EditorCamera* overrideCamera = nullptr);
		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnViewportResize(uint32_t width, uint32_t height);
		void MarkViewportDirty() { m_ViewportDirty = true; }

		bool IsViewportDirty() { return m_ViewportDirty; }
		Entity GetPrimaryCameraEntity();

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	
	private:
		entt::registry m_Registry;
		eastl::hash_map<UUID, entt::entity> m_EntityMap;
		
		b2World* m_PhysicsWorld2D;
		ContactListener* m_ContactListener;
		eastl::hash_map<b2Fixture*, entt::entity> m_FixtureMap;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		bool m_ViewportDirty = true;
		
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
		friend class ContactListener;

	public:
		uint32_t VelocityIterations = 8;
		uint32_t PositionIterations = 3;
	};
}
