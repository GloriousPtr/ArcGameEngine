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
				if (!script.Handle)
					continue;

				ScriptEngine::ReleaseObjectReference(script.Handle);
				script.Handle = nullptr;
			}
		}
	}

	template<typename Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, eastl::unordered_map<UUID, entt::entity> enttMap)
	{
		auto view = src.view<Component>();
		for (auto e : view)
		{
			UUID uuid = src.get<IDComponent>(e).ID;
			entt::entity dstEnttID = enttMap.at(uuid);
			auto& component = src.get<Component>(e);
			dst.emplace_or_replace<Component>(dstEnttID, component);
		}
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

		CopyComponent<TransformComponent>(dstRegistry, srcRegistry, newScene->m_EntityMap);
		CopyComponent<RelationshipComponent>(dstRegistry, srcRegistry, newScene->m_EntityMap);
		CopyComponent<SpriteRendererComponent>(newScene->m_Registry, srcRegistry, newScene->m_EntityMap);
		CopyComponent<CameraComponent>(newScene->m_Registry, srcRegistry, newScene->m_EntityMap);
		CopyComponent<NativeScriptComponent>(newScene->m_Registry, srcRegistry, newScene->m_EntityMap);
		CopyComponent<MeshComponent>(newScene->m_Registry, srcRegistry, newScene->m_EntityMap);
		CopyComponent<SkyLightComponent>(newScene->m_Registry, srcRegistry, newScene->m_EntityMap);
		CopyComponent<LightComponent>(newScene->m_Registry, srcRegistry, newScene->m_EntityMap);
		CopyComponent<Rigidbody2DComponent>(newScene->m_Registry, srcRegistry, newScene->m_EntityMap);
		CopyComponent<BoxCollider2DComponent>(newScene->m_Registry, srcRegistry, newScene->m_EntityMap);
		CopyComponent<CircleCollider2DComponent>(newScene->m_Registry, srcRegistry, newScene->m_EntityMap);
		CopyComponent<ScriptComponent>(newScene->m_Registry, srcRegistry, newScene->m_EntityMap);

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

		ScriptEngine::SetScene(this);

		auto scriptView = m_Registry.view<IDComponent, TagComponent, ScriptComponent>();
		constexpr char* onCreateDesc = "OnCreate()";
		constexpr char* onUpdateDesc = "OnUpdate(single)";
		constexpr char* onDestroyDesc = "OnDestroy()";
		for (auto e : scriptView)
		{
			auto& [id, tag, script] = scriptView.get<IDComponent, TagComponent, ScriptComponent>(e);
			const char* className = script.ClassName.c_str();

			if (!ScriptEngine::HasClass(className))
			{
				ARC_CORE_ERROR("{0} class not found but is attached to entity: {1}({2})", className, tag.Tag, id.ID);
				continue;
			}

			if (!script.Handle)
				continue;

			script.Handle = ScriptEngine::CopyStrongReference(script.Handle);

			void* args[] { &id.ID };
			void* property = ScriptEngine::GetProperty(className, "ID");
			ScriptEngine::SetProperty(script.Handle, property, args);
			
			ScriptEngine::CacheMethodIfAvailable(className, onCreateDesc);
			ScriptEngine::CacheMethodIfAvailable(className, onUpdateDesc);
			ScriptEngine::CacheMethodIfAvailable(className, onDestroyDesc);

			if (ScriptEngine::GetCachedMethodIfAvailable(className, onCreateDesc))
				ScriptEngine::Call(script.Handle, className, onCreateDesc, nullptr);
		}
	}

	void Scene::OnRuntimeStop()
	{
		ARC_PROFILE_SCOPE();

		auto scriptView = m_Registry.view<ScriptComponent>();
		constexpr char* onDestroyDesc = "OnDestroy()";
		for (auto e : scriptView)
		{
			ScriptComponent& script = scriptView.get<ScriptComponent>(e);
			const char* className = script.ClassName.c_str();

			if (ScriptEngine::GetCachedMethodIfAvailable(className, onDestroyDesc))
				ScriptEngine::Call(script.Handle, className, onDestroyDesc, nullptr);

			ScriptEngine::ReleaseObjectReference(script.Handle);
			script.Handle = nullptr;
		}

		ScriptEngine::SetScene(nullptr);

		delete m_PhysicsWorld2D;
		m_PhysicsWorld2D = nullptr;
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera, Ref<RenderGraphData>& renderGraphData)
	{
		ARC_PROFILE_SCOPE();
		
		// Lights
		eastl::vector<Entity> lights;
		lights.reserve(25);
		{
			ARC_PROFILE_SCOPE("PrepareLightData");

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

	void Scene::OnUpdateRuntime(Timestep ts, Ref<RenderGraphData>& renderGraphData)
	{
		ARC_PROFILE_SCOPE();

		{
			ARC_PROFILE_SCOPE("Update");

			/////////////////////////////////////////////////////////////////////
			// Scripting ////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////
			{
				void* args[1];
				float timestep = ts;
				args[0] = &timestep;
				constexpr char* onUpdateDesc = "OnUpdate(single)";
				auto scriptView = m_Registry.view<ScriptComponent>();
				for (auto e : scriptView)
				{
					ScriptComponent& script = scriptView.get<ScriptComponent>(e);
					const char* className = script.ClassName.c_str();
				
					if (ScriptEngine::GetCachedMethodIfAvailable(className, onUpdateDesc))
						ScriptEngine::Call(script.Handle, className, onUpdateDesc, args);
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
		// Rendering ////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////
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
			eastl::vector<Entity> lights;
			lights.reserve(25);
			{
				ARC_PROFILE_SCOPE("Prepare Light Data");

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
}

