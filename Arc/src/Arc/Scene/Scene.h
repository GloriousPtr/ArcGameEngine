#pragma once

#include <entt.hpp>

#include "Arc/Core/UUID.h"
#include "Arc/Core/Timestep.h"

class b2World;
class b2Fixture;

namespace ArcEngine
{
	class Entity;
	class EditorCamera;
	struct CameraData;
	struct RenderGraphData;

	struct TransformComponent;

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
		std::string Name = "Layer";
		EntityLayer Flags = 0xFFFF;
		uint8_t Index = 1;
	};

	class Scene
	{
	public:
		uint32_t VelocityIterations = 8;
		uint32_t PositionIterations = 3;
		glm::vec2 Gravity = { 0.0f, -9.8f };

		static constexpr EntityLayer StaticLayer  = BIT(0);
		static constexpr EntityLayer DefaultLayer = BIT(1);
		static std::map<EntityLayer, EntityLayerData> LayerCollisionMask;

	public:
		[[nodiscard]] static Ref<Scene> CopyTo(const Ref<Scene>& other);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		void DestroyEntity(Entity entity);
		Entity Duplicate(Entity entity);
		[[nodiscard]] bool HasEntity(UUID uuid) const;
		[[nodiscard]] Entity GetEntity(UUID uuid);
		[[nodiscard]] bool IsRunning() const { return m_IsRunning; }

		void OnUpdateEditor([[maybe_unused]] Timestep ts, const Ref<RenderGraphData>& renderGraphData, const EditorCamera& camera);
		void OnUpdateRuntime([[maybe_unused]] Timestep ts, const Ref<RenderGraphData>& renderGraphData, const EditorCamera* overrideCamera = nullptr);
		void OnRender(const Ref<RenderGraphData>& renderGraphData, const CameraData& cameraData);
		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnViewportResize(uint32_t width, uint32_t height);
		void MarkViewportDirty() { m_ViewportDirty = true; }
		[[nodiscard]] bool IsViewportDirty() const { return m_ViewportDirty; }
		[[nodiscard]] Entity GetPrimaryCameraEntity();
		void SortForSprites();

		template<typename... Components>
		[[nodiscard]] auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

	private:
		void CreateRigidbody(Entity entity, const TransformComponent& transform, RigidbodyComponent& component) const;
		void CreateRigidbody2D(Entity entity, const TransformComponent& transform, Rigidbody2DComponent& component) const;
		void CreateBoxCollider2D(Entity entity, const TransformComponent& transform, const Rigidbody2DComponent& rb, BoxCollider2DComponent& component) const;
		void CreateCircleCollider2D(Entity entity, const TransformComponent& transform, const Rigidbody2DComponent& rb, CircleCollider2DComponent& component) const;
		void CreatePolygonCollider2D(Entity entity, const Rigidbody2DComponent& rb, PolygonCollider2DComponent& component) const;

		template<typename T>
		void OnComponentAdded([[maybe_unused]] Entity entity, [[maybe_unused]] T& component);
	
	private:

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;

		entt::registry m_Registry;
		std::unordered_map<UUID, entt::entity> m_EntityMap;
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
