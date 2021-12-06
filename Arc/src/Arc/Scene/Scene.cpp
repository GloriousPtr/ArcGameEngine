#include "arcpch.h"
#include "Arc/Scene/Scene.h"

#include "Arc/Scene/Components.h"
#include "Arc/Scene/ScriptableEntity.h"
#include "Arc/Renderer/Renderer2D.h"
#include "Arc/Renderer/Renderer3D.h"

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

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		m_EntityMap.emplace(uuid, entity);

		entity.AddComponent<IDComponent>(uuid);

		entity.AddComponent<TransformComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		return entity;
	}



	void Scene::DestroyEntity(Entity entity)
	{
		entity.Deparent();

		auto& children = entity.GetTransform().Children;
		for (size_t i = 0; i < children.size(); i++)
			DestroyEntity(GetEntity(children[i]));

		m_EntityMap.erase(entity.GetUUID());
		m_Registry.destroy(entity);
	}

	bool Scene::HasEntity(UUID uuid)
	{
		return m_EntityMap.find(uuid) != m_EntityMap.end();
	}

	Entity Scene::GetEntity(UUID uuid)
	{
		if (m_EntityMap.find(uuid) != m_EntityMap.end())
			return { m_EntityMap[uuid], this };

		return {};
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);
		{
			auto view = m_Registry.view<IDComponent, SpriteRendererComponent>();
			for (auto entity : view)
			{
				auto [id, sprite] = view.get<IDComponent, SpriteRendererComponent>(entity);
				Renderer2D::DrawQuad((uint32_t)entity, GetEntity(id.ID).GetWorldTransform(), sprite.Texture, sprite.Color, sprite.TilingFactor);
			}
		}
		Renderer2D::EndScene();

		Renderer3D::BeginScene(camera);
		{
			auto view = m_Registry.view<IDComponent, MeshComponent>();
			for (auto entity : view)
			{
				auto [id, mesh] = view.get<IDComponent, MeshComponent>(entity);
				if (mesh.VertexArray != nullptr)
					Renderer3D::DrawMesh((uint32_t)entity, mesh.VertexArray, mesh.DiffuseMap, GetEntity(id.ID).GetWorldTransform());
			}
		}
		Renderer3D::EndScene();
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
			auto view = m_Registry.view<IDComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto [id, camera] = view.get<IDComponent, CameraComponent>(entity);

				if(camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = GetEntity(id.ID).GetWorldTransform();
					break;
				}
			}
		}

		if(mainCamera)
		{
			Renderer2D::BeginScene(*mainCamera, cameraTransform);
			
			auto view = m_Registry.view<IDComponent, SpriteRendererComponent>();
			for (auto entity : view)
			{
				auto [id, sprite] = view.get<IDComponent, SpriteRendererComponent>(entity);
				
				Renderer2D::DrawQuad((uint32_t)entity, GetEntity(id.ID).GetWorldTransform(), sprite.Texture, sprite.Color, sprite.TilingFactor);
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

		m_ViewportDirty = false;
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
	void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component)
	{
	}
}

