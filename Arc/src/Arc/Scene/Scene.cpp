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
		OPTICK_EVENT();

		ARC_PROFILE_FUNCTION();

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
		OPTICK_EVENT();

		ARC_PROFILE_FUNCTION();

		entity.Deparent();
		auto children = entity.GetTransform().Children;

		for (size_t i = 0; i < children.size(); i++)
			DestroyEntity(GetEntity(children[i]));

		m_EntityMap.erase(entity.GetUUID());
		m_Registry.destroy(entity);
	}

	bool Scene::HasEntity(UUID uuid)
	{
		OPTICK_EVENT();

		ARC_PROFILE_FUNCTION();

		return m_EntityMap.find(uuid) != m_EntityMap.end();
	}

	Entity Scene::GetEntity(UUID uuid)
	{
		OPTICK_EVENT();

		ARC_PROFILE_FUNCTION();

		if (m_EntityMap.find(uuid) != m_EntityMap.end())
			return { m_EntityMap.at(uuid), this };

		return {};
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera, Ref<RenderGraphData>& renderGraphData)
	{
		OPTICK_EVENT();

		ARC_PROFILE_FUNCTION();
		
		// Lights
		std::vector<Entity> lights;
		lights.reserve(25);
		{
			OPTICK_EVENT("PrepareLightData");

			auto view = m_Registry.view<IDComponent, LightComponent>();
			lights.reserve(view.size());
			for (auto entity : view)
			{
				auto [id, light] = view.get<IDComponent, LightComponent>(entity);
				lights.push_back(Entity(entity, this));
			}
		}
		Entity skylight = {};
		{
			OPTICK_EVENT("PrepareSkylightData");

			auto view = m_Registry.view<IDComponent, SkyLightComponent>();
			for (auto entity : view)
			{
				auto [id, light] = view.get<IDComponent, SkyLightComponent>(entity);
				skylight = Entity(entity, this);
			}
		}

		Renderer3D::BeginScene(camera, skylight, lights);
		// Meshes
		{
			OPTICK_EVENT("Submit Mesh Data");

			auto view = m_Registry.view<IDComponent, MeshComponent>();
			for (auto entity : view)
			{
				auto [id, mesh] = view.get<IDComponent, MeshComponent>(entity);
				if (mesh.VertexArray != nullptr)
					Renderer3D::SubmitMesh(mesh, Entity(entity, this).GetWorldTransform());
			}
		}
		Renderer3D::EndScene(renderGraphData);
		
		Renderer2D::BeginScene(camera);
		{
			OPTICK_EVENT("Submit 2D Data");

			auto view = m_Registry.view<IDComponent, SpriteRendererComponent>();
			for (auto entity : view)
			{
				auto [id, sprite] = view.get<IDComponent, SpriteRendererComponent>(entity);
				Renderer2D::DrawQuad(Entity(entity, this).GetWorldTransform(), sprite.Texture, sprite.Color, sprite.TilingFactor);
			}
		}
		Renderer2D::EndScene(renderGraphData);
	}

	void Scene::OnUpdateRuntime(Timestep ts, Ref<RenderGraphData>& renderGraphData)
	{
		OPTICK_EVENT();

		ARC_PROFILE_FUNCTION();

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
				
				Renderer2D::DrawQuad(GetEntity(id.ID).GetWorldTransform(), sprite.Texture, sprite.Color, sprite.TilingFactor);
			}

			Renderer2D::EndScene(renderGraphData);
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		OPTICK_EVENT();

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

	Entity Scene::GetPrimaryCameraEntity()
	{
		OPTICK_EVENT();

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

	template<>
	void Scene::OnComponentAdded<SkyLightComponent>(Entity entity, SkyLightComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<LightComponent>(Entity entity, LightComponent& component)
	{
	}
}

