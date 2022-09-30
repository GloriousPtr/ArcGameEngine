#pragma once

#include <EASTL/map.h>

#include "Arc/Core/UUID.h"
#include "Arc/Core/Timestep.h"
#include "Arc/Renderer/EditorCamera.h"
#include "Arc/Renderer/RenderGraphData.h"

#include "entt.hpp"

class b2World;
class b2Fixture;

namespace ArcEngine
{
	class Entity;

	class Physics3DContactListener;
	class Physics3DBodyActivationListener;
	struct RigidbodyComponent;
	struct BoxColliderComponent;
	struct SphereColliderComponent;

	class Physics2DContactListener;
	struct Rigidbody2DComponent;
	struct BoxCollider2DComponent;
	struct CircleCollider2DComponent;
	struct PolygonCollider2DComponent;
	
	using EntityLayer = uint16_t;
	struct EntityLayerData
	{
		eastl::string Name = "Layer";
		EntityLayer Flags = 0xFFFF;
		uint8_t Index = 1;
	};

	class Scene
	{
	public:
		uint32_t VelocityIterations = 8;
		uint32_t PositionIterations = 3;
		glm::vec2 Gravity = { 0.0f, -9.8f };

		static const EntityLayer StaticLayer  = BIT(0);
		static const EntityLayer DefaultLayer = BIT(1);
		static eastl::map<EntityLayer, EntityLayerData> LayerCollisionMask;

	public:
		Scene() = default;
		~Scene();

		static Ref<Scene> CopyTo(Ref<Scene> other);

		Entity CreateEntity(const eastl::string& name = eastl::string());
		Entity CreateEntityWithUUID(UUID uuid, const eastl::string& name = eastl::string());
		void DestroyEntity(Entity entity);
		Entity Duplicate(Entity entity);
		bool HasEntity(UUID uuid);
		Entity GetEntity(UUID uuid);
		bool IsRunning() const { return m_IsRunning; }

		void OnUpdateEditor([[maybe_unused]] Timestep ts, const Ref<RenderGraphData>& renderGraphData, const EditorCamera& camera);
		void OnUpdateRuntime([[maybe_unused]] Timestep ts, const Ref<RenderGraphData>& renderGraphData, const EditorCamera* overrideCamera = nullptr);
		void OnRender(const Ref<RenderGraphData>& renderGraphData, const CameraData& cameraData);
		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnViewportResize(uint32_t width, uint32_t height);
		void MarkViewportDirty() { m_ViewportDirty = true; }
		bool IsViewportDirty() const { return m_ViewportDirty; }
		Entity GetPrimaryCameraEntity();

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

	private:
		void CreateRigidbody(Entity entity, RigidbodyComponent& component) const;
		void CreateRigidbody2D(Entity entity, Rigidbody2DComponent& component);
		void CreateBoxCollider2D(Entity entity, BoxCollider2DComponent& component);
		void CreateCircleCollider2D(Entity entity, CircleCollider2DComponent& component);
		void CreatePolygonCollider2D(Entity entity, PolygonCollider2DComponent& component);

		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	
	private:

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;

		entt::registry m_Registry;
		eastl::hash_map<UUID, entt::entity> m_EntityMap;
		bool m_IsRunning = false;

		b2World* m_PhysicsWorld2D = nullptr;
		Physics2DContactListener* m_ContactListener2D = nullptr;
		Physics3DContactListener* m_ContactListener3D = nullptr;
		Physics3DBodyActivationListener* m_BodyActivationListener3D = nullptr;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		bool m_ViewportDirty = true;

		float m_PhysicsFrameAccumulator = 0.0f;
	};
}
