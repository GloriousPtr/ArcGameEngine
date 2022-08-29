#include "arcpch.h"
#include "Arc/Scene/Scene.h"

#include "Arc/Scene/Components.h"
#include "Arc/Scene/ScriptableEntity.h"
#include "Arc/Renderer/Renderer2D.h"
#include "Arc/Renderer/Renderer3D.h"
#include "Arc/Scripting/ScriptEngine.h"

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <box2d/box2d.h>

#include "Entity.h"

namespace ArcEngine
{
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
		ARC_PROFILE_SCOPE();

		// Scripting
		{
			auto scriptView = m_Registry.view<ScriptComponent>();
			for (auto e : scriptView)
			{
				ScriptComponent& script = scriptView.get<ScriptComponent>(e);
				Entity entity = { e, this };
				for (auto& className : script.Classes)
					ScriptEngine::RemoveInstance(entity, className);
			}
		}
	}

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, eastl::hash_map<UUID, entt::entity> enttMap)
	{
		([&]()
		{
			auto view = src.view<Component>();
			for (auto e : view)
			{
				UUID uuid = src.get<IDComponent>(e).ID;
				entt::entity dstEnttID = enttMap.at(uuid);
				auto& component = src.get<Component>(e);
				dst.emplace_or_replace<Component>(dstEnttID, component);
			}
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, eastl::hash_map<UUID, entt::entity> enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	Ref<Scene> Scene::CopyTo(Ref<Scene> other)
	{
		ARC_PROFILE_SCOPE();

		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		auto& srcRegistry = other->m_Registry;
		auto& dstRegistry = newScene->m_Registry;
		auto view = other->m_Registry.view<IDComponent, TagComponent>();
		for (auto e : view)
		{
			auto [id, tag] = view.get<IDComponent, TagComponent>(e);
			newScene->CreateEntityWithUUID(id.ID, tag.Tag);
		}

		for (auto e : view)
		{
			auto id = view.get<IDComponent>(e).ID;
			Entity src = { e, other.get() };
			Entity dst = newScene->GetEntity(id);
			Entity srcParent = src.GetParent();
			if (srcParent)
			{
				Entity dstParent = newScene->GetEntity(srcParent.GetUUID());
				dst.SetParent(dstParent);
			}
		}

		CopyComponent(AllComponents{}, dstRegistry, srcRegistry, newScene->m_EntityMap);

		return newScene;
	}

	Entity Scene::CreateEntity(const eastl::string& name)
	{
		ARC_PROFILE_SCOPE();

		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const eastl::string& name)
	{
		ARC_PROFILE_SCOPE();

		Entity entity = { m_Registry.create(), this };
		m_EntityMap.emplace(uuid, entity);

		entity.AddComponent<IDComponent>(uuid);

		entity.AddComponent<RelationshipComponent>();
		entity.AddComponent<TransformComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		ARC_PROFILE_SCOPE();

		entity.Deparent();
		auto children = entity.GetComponent<RelationshipComponent>().Children;

		for (size_t i = 0; i < children.size(); i++)
			DestroyEntity(GetEntity(children[i]));

		m_EntityMap.erase(entity.GetUUID());
		m_Registry.destroy(entity);
	}

	bool Scene::HasEntity(UUID uuid)
	{
		ARC_PROFILE_SCOPE();

		return m_EntityMap.find(uuid) != m_EntityMap.end();
	}

	Entity Scene::GetEntity(UUID uuid)
	{
		ARC_PROFILE_SCOPE();

		if (m_EntityMap.find(uuid) != m_EntityMap.end())
			return { m_EntityMap.at(uuid), this };

		return {};
	}

	void Scene::OnRuntimeStart()
	{
		ARC_PROFILE_SCOPE();

		m_PhysicsWorld2D = new b2World({ 0.0f, -9.8f });
		auto view = m_Registry.view<TransformComponent, Rigidbody2DComponent>();
		for (auto e : view)
		{
			auto [transform, body] = view.get<TransformComponent, Rigidbody2DComponent>(e);
			
			b2BodyDef def;
			def.type = (b2BodyType)body.Type;
			def.linearDamping = body.LinearDrag;
			def.angularDamping = body.AngularDrag;
			def.allowSleep = body.AllowSleep;
			def.awake = body.Awake;
			def.fixedRotation = body.FreezeRotation;
			def.bullet = body.Continuous;
			def.gravityScale = body.GravityScale;

			def.position.Set(transform.Translation.x, transform.Translation.y);
			def.angle = transform.Rotation.z;

			b2Body* rb = m_PhysicsWorld2D->CreateBody(&def);
			body.RuntimeBody = rb;

			Entity entity = { e, this };
			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

				b2PolygonShape boxShape;
				boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2d.Density;
				fixtureDef.friction = bc2d.Friction;
				fixtureDef.restitution = bc2d.Restitution;
				fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;

				bc2d.RuntimeFixture = rb->CreateFixture(&fixtureDef);
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

				b2CircleShape circleShape;
				circleShape.m_radius = cc2d.Radius * glm::max(transform.Scale.x, transform.Scale.y);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.density = cc2d.Density;
				fixtureDef.friction = cc2d.Friction;
				fixtureDef.restitution = cc2d.Restitution;
				fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;

				cc2d.RuntimeFixture = rb->CreateFixture(&fixtureDef);
			}

			if (!body.AutoMass && body.Mass > 0.01f)
			{
				b2MassData massData = rb->GetMassData();
				massData.mass = body.Mass;
				rb->SetMassData(&massData);
			}
		}


		/////////////////////////////////////////////////////////////////////
		// Sound ////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////
		{
			ARC_PROFILE_SCOPE("Sound");

			bool foundActive = false;
			auto listenerView = m_Registry.view<TransformComponent, AudioListenerComponent>();
			for (auto e : listenerView)
			{
				auto [tc, ac] = listenerView.get<TransformComponent, AudioListenerComponent>(e);
				ac.Listener = CreateRef<AudioListener>();
				if (ac.Active && !foundActive)
				{
					foundActive = true;
					Entity entity = { e, this };
					const glm::mat4 inverted = glm::inverse(entity.GetWorldTransform());
					const glm::vec3 forward = normalize(glm::vec3(inverted[2]));
					ac.Listener->SetConfig(ac.Config);
					ac.Listener->SetPosition(tc.Translation);
					ac.Listener->SetDirection(-forward);
				}
			}

			auto sourceView = m_Registry.view<TransformComponent, AudioSourceComponent>();
			for (auto e : sourceView)
			{
				auto [tc, ac] = sourceView.get<TransformComponent, AudioSourceComponent>(e);
				if (ac.Source)
				{
					Entity entity = { e, this };
					const glm::mat4 inverted = glm::inverse(entity.GetWorldTransform());
					const glm::vec3 forward = normalize(glm::vec3(inverted[2]));
					ac.Source->SetConfig(ac.Config);
					ac.Source->SetPosition(tc.Translation);
					ac.Source->SetDirection(forward);
					if (ac.Config.PlayOnAwake)
						ac.Source->Play();
				}
			}
		}

		/////////////////////////////////////////////////////////////////////
		// Scripting ////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////
		ScriptEngine::SetScene(this);
		ScriptEngine::OnRuntimeBegin();
		auto scriptView = m_Registry.view<IDComponent, TagComponent, ScriptComponent>();
		for (auto e : scriptView)
		{
			auto& [id, tag, script] = scriptView.get<IDComponent, TagComponent, ScriptComponent>(e);
			Entity entity = { e, this };

			for (auto& className : script.Classes)
				ScriptEngine::GetInstance(entity, className)->InvokeOnCreate();
		}
	}

	void Scene::OnRuntimeStop()
	{
		ARC_PROFILE_SCOPE();

		auto scriptView = m_Registry.view<ScriptComponent>();
		for (auto e : scriptView)
		{
			ScriptComponent& script = scriptView.get<ScriptComponent>(e);
			Entity entity = { e, this };

			for (auto& className : script.Classes)
			{
				ScriptEngine::GetInstance(entity, className)->InvokeOnDestroy();
				ScriptEngine::RemoveInstance(entity, className);
			}

			script.Classes.clear();
		}
		ScriptEngine::OnRuntimeEnd();
		ScriptEngine::SetScene(nullptr);

		/////////////////////////////////////////////////////////////////////
		// Sound ////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////
		{
			ARC_PROFILE_SCOPE("Sound");

			auto view = m_Registry.view<TransformComponent, AudioSourceComponent>();
			for (auto e : view)
			{
				auto [tc, ac] = view.get<TransformComponent, AudioSourceComponent>(e);
				if (ac.Source)
					ac.Source->Stop();
			}
		}

		delete m_PhysicsWorld2D;
		m_PhysicsWorld2D = nullptr;
	}

	void Scene::OnUpdateEditor(Timestep ts, Ref<RenderGraphData>& renderGraphData, EditorCamera& camera)
	{
		ARC_PROFILE_SCOPE();
		
		// Lights
		eastl::vector<Entity> lights;
		lights.reserve(Renderer3D::MAX_NUM_LIGHTS);
		{
			ARC_PROFILE_SCOPE("PrepareLightData");

			auto view = m_Registry.view<IDComponent, LightComponent>();
			lights.reserve(view.size());
			for (auto entity : view)
			{
				auto [id, light] = view.get<IDComponent, LightComponent>(entity);
				lights.emplace_back(Entity(entity, this));
			}
		}
		Entity skylight = {};
		{
			ARC_PROFILE_SCOPE("PrepareSkylightData");

			auto view = m_Registry.view<IDComponent, SkyLightComponent>();
			for (auto entity : view)
			{
				auto [id, light] = view.get<IDComponent, SkyLightComponent>(entity);
				skylight = Entity(entity, this);
				break;
			}
		}

		Renderer3D::BeginScene(camera, skylight, lights);
		// Meshes
		{
			ARC_PROFILE_SCOPE("Submit Mesh Data");

			auto view = m_Registry.view<IDComponent, MeshComponent>();
			Renderer3D::ReserveMeshes(view.size());
			for (auto entity : view)
			{
				auto [id, mesh] = view.get<IDComponent, MeshComponent>(entity);
				if (mesh.MeshGeometry != nullptr)
					Renderer3D::SubmitMesh(mesh, Entity(entity, this).GetWorldTransform());
			}
		}
		Renderer3D::EndScene(renderGraphData);
		
		Renderer2D::BeginScene(camera);
		{
			ARC_PROFILE_SCOPE("Submit 2D Data");

			auto view = m_Registry.view<IDComponent, SpriteRendererComponent>();
			for (auto entity : view)
			{
				auto [id, sprite] = view.get<IDComponent, SpriteRendererComponent>(entity);
				Renderer2D::DrawQuad(Entity(entity, this).GetWorldTransform(), sprite.Texture, sprite.Color, sprite.TilingFactor);
			}
		}
		Renderer2D::EndScene(renderGraphData);
	}

	void Scene::OnUpdateRuntime(Timestep ts, Ref<RenderGraphData>& renderGraphData, EditorCamera* overrideCamera)
	{
		ARC_PROFILE_SCOPE();

		{
			ARC_PROFILE_SCOPE("Update");

			/////////////////////////////////////////////////////////////////////
			// Scripting ////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////
			{
				auto scriptView = m_Registry.view<ScriptComponent>();
				for (auto e : scriptView)
				{
					ScriptComponent& script = scriptView.get<ScriptComponent>(e);
					Entity entity = { e, this };

					for (auto& className : script.Classes)
						ScriptEngine::GetInstance(entity, className)->InvokeOnUpdate(ts);
				}
			}

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
		}
		
		/////////////////////////////////////////////////////////////////////
		// Physics //////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////
		{
			ARC_PROFILE_SCOPE("Physics 2D");

			auto view = m_Registry.view<TransformComponent, Rigidbody2DComponent>();
			for (auto e : view)
			{
				auto [tc, body] = view.get<TransformComponent, Rigidbody2DComponent>(e);
				b2Body* rb = (b2Body*)body.RuntimeBody;
				rb->SetTransform(b2Vec2(tc.Translation.x, tc.Translation.y), tc.Rotation.z);
			}

			m_PhysicsWorld2D->Step(ts, VelocityIterations, PositionIterations);
		
			for (auto e : view)
			{
				auto [transform, body] = view.get<TransformComponent, Rigidbody2DComponent>(e);
				b2Body* rb = (b2Body*)body.RuntimeBody;
				b2Vec2 position = rb->GetPosition();
				transform.Translation.x = position.x;
				transform.Translation.y = position.y;
				transform.Rotation.z = rb->GetAngle();
			}
		}

		/////////////////////////////////////////////////////////////////////
		// Sound ////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////
		{
			ARC_PROFILE_SCOPE("Sound");

			auto listenerView = m_Registry.view<TransformComponent, AudioListenerComponent>();
			for (auto e : listenerView)
			{
				auto [tc, ac] = listenerView.get<TransformComponent, AudioListenerComponent>(e);
				if (ac.Active)
				{
					Entity entity = { e, this };
					const glm::mat4 inverted = glm::inverse(entity.GetWorldTransform());
					const glm::vec3 forward = normalize(glm::vec3(inverted[2]));
					ac.Listener->SetPosition(tc.Translation);
					ac.Listener->SetDirection(-forward);
					break;
				}
			}

			auto sourceView = m_Registry.view<TransformComponent, AudioSourceComponent>();
			for (auto e : sourceView)
			{
				auto [tc, ac] = sourceView.get<TransformComponent, AudioSourceComponent>(e);
				if (ac.Source)
				{
					Entity entity = { e, this };
					const glm::mat4 inverted = glm::inverse(entity.GetWorldTransform());
					const glm::vec3 forward = normalize(glm::vec3(inverted[2]));
					ac.Source->SetPosition(tc.Translation);
					ac.Source->SetDirection(forward);
				}
			}
		}

		/////////////////////////////////////////////////////////////////////
		// Rendering ////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////

		glm::mat4 cameraTransform;
		Camera* mainCamera = nullptr;
		if (!overrideCamera)
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
		else
		{
			mainCamera = overrideCamera;
			cameraTransform = glm::inverse(overrideCamera->GetViewMatrix());
		}

		if(mainCamera)
		{
			eastl::vector<Entity> lights;
			lights.reserve(Renderer3D::MAX_NUM_LIGHTS);
			{
				ARC_PROFILE_SCOPE("Prepare Light Data");

				auto view = m_Registry.view<IDComponent, LightComponent>();
				lights.reserve(view.size());
				for (auto entity : view)
				{
					auto [id, light] = view.get<IDComponent, LightComponent>(entity);
					lights.emplace_back(Entity(entity, this));
				}
			}
			Entity skylight = {};
			{
				ARC_PROFILE_SCOPE("Prepare Skylight Data");

				auto view = m_Registry.view<IDComponent, SkyLightComponent>();
				for (auto entity : view)
				{
					auto [id, light] = view.get<IDComponent, SkyLightComponent>(entity);
					skylight = Entity(entity, this);
					break;
				}
			}

			Renderer3D::BeginScene(*mainCamera, cameraTransform, skylight, lights);
			// Meshes
			{
				ARC_PROFILE_SCOPE("Submit Mesh Data");

				auto view = m_Registry.view<IDComponent, MeshComponent>();
				Renderer3D::ReserveMeshes(view.size());
				for (auto entity : view)
				{
					auto [id, mesh] = view.get<IDComponent, MeshComponent>(entity);
					if (mesh.MeshGeometry != nullptr)
						Renderer3D::SubmitMesh(mesh, Entity(entity, this).GetWorldTransform());
				}
			}
			Renderer3D::EndScene(renderGraphData);

			Renderer2D::BeginScene(*mainCamera, cameraTransform);
			{
				ARC_PROFILE_SCOPE("Submit 2D Data");

				auto view = m_Registry.view<IDComponent, SpriteRendererComponent>();
				for (auto entity : view)
				{
					auto [id, sprite] = view.get<IDComponent, SpriteRendererComponent>(entity);
				
					Renderer2D::DrawQuad(GetEntity(id.ID).GetWorldTransform(), sprite.Texture, sprite.Color, sprite.TilingFactor);
				}
			}
			Renderer2D::EndScene(renderGraphData);
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		ARC_PROFILE_SCOPE();

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
		ARC_PROFILE_SCOPE();

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
	void Scene::OnComponentAdded<RelationshipComponent>(Entity entity, RelationshipComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<PrefabComponent>(Entity entity, PrefabComponent& component)
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

	template<>
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
	{
	}
	
	template<>
	void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<AudioSourceComponent>(Entity entity, AudioSourceComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<AudioListenerComponent>(Entity entity, AudioListenerComponent& component)
	{
	}
}
