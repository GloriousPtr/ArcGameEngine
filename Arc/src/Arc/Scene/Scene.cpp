#include "arcpch.h"
#include "Arc/Scene/Scene.h"

#include "Arc/Scene/Components.h"
#include "Arc/Renderer/Renderer2D.h"
#include "Arc/Physics/Physics2D.h"

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "Entity.h"

namespace ArcEngine
{
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	template<typename T>
	static void CopyComponent(entt::registry& destRegistry, entt::registry& srcRegistry, const std::unordered_map<uint32_t, entt::entity>& entityMap)
	{
		auto components = srcRegistry.view<T>();
		for (auto srcEntity : components)
		{
			entt::entity destEntity = entityMap.at(srcRegistry.get<IDComponent>(srcEntity).ID);

			auto& srcComponent = srcRegistry.get<T>(srcEntity);
			auto& destComponent = destRegistry.emplace<T>(destEntity, srcComponent);
		}
	}

	void Scene::CopyTo(Ref<Scene>& target)
	{
		std::unordered_map<uint32_t, entt::entity> enttMap;
		auto idComponents = m_Registry.view<IDComponent>();
		for (auto entity : idComponents)
		{
			auto uuid = m_Registry.get<IDComponent>(entity).ID;
			Entity e = target->CreateEntityWithID(uuid);
			enttMap[uuid] = e;
		}

		CopyComponent<TagComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<TransformComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<CameraComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<SpriteRendererComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<Rigidbody2DComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<BoxCollider2DComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<NativeScriptComponent>(target->m_Registry, m_Registry, enttMap);
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		uint32_t id = entity;
		auto& idComponent = entity.AddComponent<IDComponent>(id);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		m_EntityMap[idComponent.ID] = entity;

		return entity;
	}

	Entity Scene::CreateEntityWithID(const uint32_t id)
	{
		Entity entity = { m_Registry.create(), this };
		auto& idComponent = entity.AddComponent<IDComponent>(id);

		m_EntityMap[idComponent.ID] = entity;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);

		{
			auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
			for (auto entity : view)
			{
				auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawQuad((uint32_t)entity, transform.GetTransform(), sprite.Texture, sprite.Color, sprite.TilingFactor);
			}
		}

		Renderer2D::EndScene();
	}

	void Scene::OnRuntimeStart()
	{
		Physics2D::Init();

		{
			auto view = m_Registry.view<TransformComponent, Rigidbody2DComponent>();
			for (auto entity : view)
			{
				auto [transform, rigidbody2d] = view.get<TransformComponent, Rigidbody2DComponent>(entity);
				rigidbody2d.StartSimulation(transform.Translation, transform.Rotation.z);
			}
		}

		{
			auto view = m_Registry.view<Rigidbody2DComponent, BoxCollider2DComponent>();
			for (auto entity : view)
			{
				auto [rigidbody2d, boxCollider2d] = view.get<Rigidbody2DComponent, BoxCollider2DComponent>(entity);
				boxCollider2d.StartSimulation(rigidbody2d.Body2D);
			}
		}
	}

	void Scene::OnRuntimeEnd()
	{
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		// Update Scripts
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				if(!nsc.Instance)
				{
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = { entity, this };
					
					nsc.Instance->OnCreate();
				}

				nsc.Instance->OnUpdate(ts);
			});
		}
		
		// Render 2D
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto view = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

				if(camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = transform.GetTransform();
					break;
				}
			}
		}

		if(mainCamera)
		{
			Renderer2D::BeginScene(*mainCamera, cameraTransform);
			
			{
				auto view = m_Registry.view<TransformComponent, Rigidbody2DComponent>();
				for (auto entity : view)
				{
					auto [transform, rigidbody2d] = view.get<TransformComponent, Rigidbody2DComponent>(entity);

					rigidbody2d.Body2D->SetRuntimeTransform(transform.Translation, transform.Rotation.z);
				}

				Physics2D::OnUpdate();

				for (auto entity : view)
				{
					auto [transform, rigidbody2d] = view.get<TransformComponent, Rigidbody2DComponent>(entity);

					glm::vec2 position = rigidbody2d.Body2D->GetRuntimePosition();
					transform.Translation = glm::vec3(position.x, position.y, transform.Translation.z);

					transform.Rotation = glm::vec3(transform.Rotation.x, transform.Rotation.y, rigidbody2d.Body2D->GetRuntimeRotation());
				}
			}

			{
				auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
				for (auto entity : view)
				{
					auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);

					Renderer2D::DrawQuad((uint32_t)entity, transform.GetTransform(), sprite.Texture, sprite.Color, sprite.TilingFactor);
				}
			}

			Renderer2D::EndScene();
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize our non-FixedAspectRatio cameras
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		}

	}

	int Scene::GetPixelDataAtPoint(const int x, const int y)
	{
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		int pixelData;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
		return pixelData;
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity(entity, this);
		}
		return {};
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false);
	}

	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
	{
	}
}

