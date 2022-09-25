#include "arcpch.h"
#include "Arc/Scene/Scene.h"

#include "Arc/Physics/Physics3D.h"
#include "Arc/Physics/PhysicsUtils.h"
#include "Arc/Renderer/Renderer2D.h"
#include "Arc/Renderer/Renderer3D.h"
#include "Arc/Scene/Components.h"
#include "Arc/Scripting/ScriptEngine.h"

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <box2d/box2d.h>
#include <EASTL/set.h>

#include "Entity.h"

// Jolt includes
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include "Arc/Physics/PhysicsMaterial3D.h"

namespace ArcEngine
{
	class ContactListener : public b2ContactListener
	{
	public:

		explicit ContactListener(Scene* scene)
			: m_Scene(scene)
		{
		}

		virtual ~ContactListener()
		{
			m_BuoyancyFixtures.clear();
		}

		ContactListener(const ContactListener& other) = delete;
		ContactListener(ContactListener&& other) = delete;
		ContactListener& operator=(const ContactListener& other) = delete;
		ContactListener& operator=(ContactListener&& other) = delete;

		virtual void BeginContact(b2Contact* contact) override
		{
			ARC_PROFILE_SCOPE();

			b2Fixture* a = contact->GetFixtureA();
			b2Fixture* b = contact->GetFixtureB();

			bool aSensor = a->IsSensor();
			bool bSensor = b->IsSensor();
			Entity e1 = { m_Scene->m_FixtureMap.at(a), m_Scene };
			Entity e2 = { m_Scene->m_FixtureMap.at(b), m_Scene };

			if (e1.HasComponent<BuoyancyEffector2DComponent>() && aSensor
				&& !e2.HasComponent<BuoyancyEffector2DComponent>() && b->GetBody()->GetType() == b2_dynamicBody)
			{
				m_BuoyancyFixtures.insert(eastl::make_pair(a, b));
			}
			else if (e2.HasComponent<BuoyancyEffector2DComponent>() && bSensor
				&& !e1.HasComponent<BuoyancyEffector2DComponent>() && a->GetBody()->GetType() == b2_dynamicBody)
			{
				m_BuoyancyFixtures.insert(eastl::make_pair(b, a));
			}

			b2WorldManifold worldManifold;
			contact->GetWorldManifold(&worldManifold);
			b2Vec2 point = worldManifold.points[0];
			b2Vec2 velocityA = a->GetBody()->GetLinearVelocityFromWorldPoint(point);
			b2Vec2 velocityB = b->GetBody()->GetLinearVelocityFromWorldPoint(point);

			Collision2DData collisionData;
			collisionData.EntityID = e2.GetUUID();
			collisionData.RelativeVelocity = { velocityB.x - velocityA.x, velocityB.y - velocityA.y };

			if (e1.HasComponent<ScriptComponent>())
			{
				const auto& sc = e1.GetComponent<ScriptComponent>();
				if (bSensor)
				{
					for (const auto& className : sc.Classes)
						ScriptEngine::GetInstance(e1, className)->InvokeOnSensorEnter2D(collisionData);
				}
				else
				{
					for (const auto& className : sc.Classes)
						ScriptEngine::GetInstance(e1, className)->InvokeOnCollisionEnter2D(collisionData);
				}
			}

			point = worldManifold.points[1];
			velocityA = a->GetBody()->GetLinearVelocityFromWorldPoint(point);
			velocityB = b->GetBody()->GetLinearVelocityFromWorldPoint(point);

			collisionData.EntityID = e1.GetUUID();
			collisionData.RelativeVelocity = { velocityA.x - velocityB.x, velocityA.y - velocityB.y };
			if (e2.HasComponent<ScriptComponent>())
			{
				const auto& sc = e2.GetComponent<ScriptComponent>();
				if (aSensor)
				{
					for (const auto& className : sc.Classes)
						ScriptEngine::GetInstance(e2, className)->InvokeOnSensorEnter2D(collisionData);
				}
				else
				{
					for (const auto& className : sc.Classes)
						ScriptEngine::GetInstance(e2, className)->InvokeOnCollisionEnter2D(collisionData);
				}
			}
		}

		virtual void EndContact(b2Contact* contact) override
		{
			ARC_PROFILE_SCOPE();

			b2Fixture* a = contact->GetFixtureA();
			b2Fixture* b = contact->GetFixtureB();
			bool aSensor = a->IsSensor();
			bool bSensor = b->IsSensor();
			Entity e1 = { m_Scene->m_FixtureMap.at(a), m_Scene };
			Entity e2 = { m_Scene->m_FixtureMap.at(b), m_Scene };

			if (e1.HasComponent<BuoyancyEffector2DComponent>() && aSensor
				&& !e2.HasComponent<BuoyancyEffector2DComponent>() && b->GetBody()->GetType() == b2_dynamicBody)
			{
				m_BuoyancyFixtures.erase(eastl::make_pair(a, b));
			}
			else if (e2.HasComponent<BuoyancyEffector2DComponent>() && bSensor
				&& !e1.HasComponent<BuoyancyEffector2DComponent>() && a->GetBody()->GetType() == b2_dynamicBody)
			{
				m_BuoyancyFixtures.erase(eastl::make_pair(b, a));
			}

			b2WorldManifold worldManifold;
			contact->GetWorldManifold(&worldManifold);
			b2Vec2 point = worldManifold.points[0];
			b2Vec2 velocityA = a->GetBody()->GetLinearVelocityFromWorldPoint(point);
			b2Vec2 velocityB = b->GetBody()->GetLinearVelocityFromWorldPoint(point);

			Collision2DData collisionData;
			collisionData.EntityID = e2.GetUUID();
			collisionData.RelativeVelocity = { velocityB.x - velocityA.x, velocityB.y - velocityA.y };

			if (e1.HasComponent<ScriptComponent>())
			{
				const auto& sc = e1.GetComponent<ScriptComponent>();
				if (bSensor)
				{
					for (const auto& className : sc.Classes)
						ScriptEngine::GetInstance(e1, className)->InvokeOnSensorExit2D(collisionData);
				}
				else
				{
					for (const auto& className : sc.Classes)
						ScriptEngine::GetInstance(e1, className)->InvokeOnCollisionExit2D(collisionData);
				}
			}

			point = worldManifold.points[1];
			velocityA = a->GetBody()->GetLinearVelocityFromWorldPoint(point);
			velocityB = b->GetBody()->GetLinearVelocityFromWorldPoint(point);

			collisionData.EntityID = e1.GetUUID();
			collisionData.RelativeVelocity = { velocityA.x - velocityB.x, velocityA.y - velocityB.y };
			if (e2.HasComponent<ScriptComponent>())
			{
				const auto& sc = e2.GetComponent<ScriptComponent>();
				if (aSensor)
				{
					for (const auto& className : sc.Classes)
						ScriptEngine::GetInstance(e2, className)->InvokeOnSensorExit2D(collisionData);
				}
				else
				{
					for (const auto& className : sc.Classes)
						ScriptEngine::GetInstance(e2, className)->InvokeOnCollisionExit2D(collisionData);
				}
			}
		}

		virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override
		{
			ARC_PROFILE_SCOPE();

			/* Handle pre solve */
		}

		virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override
		{
			ARC_PROFILE_SCOPE();

			/* Handle post solve */
		}

		void OnUpdate([[maybe_unused]] Timestep ts)
		{
			ARC_PROFILE_SCOPE();

			auto it = m_BuoyancyFixtures.begin();
			auto end = m_BuoyancyFixtures.end();
			while (it != end)
			{
				b2Fixture* fluid = it->first;
				b2Fixture* fixture = it->second;

				Entity fluidEntity = { m_Scene->m_FixtureMap.at(fluid), m_Scene };
				Entity fixtureEntity = { m_Scene->m_FixtureMap.at(fixture), m_Scene };

				const auto& buoyancyComponent2D = fluidEntity.GetComponent<BuoyancyEffector2DComponent>();
				PhysicsUtils::HandleBuoyancy(fluid, fixture, m_Scene->m_PhysicsWorld2D->GetGravity(), buoyancyComponent2D.FlipGravity, buoyancyComponent2D.Density, buoyancyComponent2D.DragMultiplier, buoyancyComponent2D.FlowMagnitude, buoyancyComponent2D.FlowAngle);
				++it;
			}
		}

	private:
		Scene* m_Scene;

		eastl::set<eastl::pair<b2Fixture*, b2Fixture*>> m_BuoyancyFixtures;
	};

	Scene::~Scene()
	{
		ARC_PROFILE_SCOPE();
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
				const auto& component = src.get<Component>(e);
				dst.emplace_or_replace<Component>(dstEnttID, component);
			}
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, eastl::hash_map<UUID, entt::entity> enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, Entity srcEntity, Entity dstEntity)
	{
		([&]()
		{
			if (typeid(Component) != typeid(ScriptComponent) && srcEntity.HasComponent<Component>())	// Currently not support duplicating ScriptComponent
			{
				const auto& component = srcEntity.GetComponent<Component>();
				dst.emplace_or_replace<Component>(dstEntity, component);
			}
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, Entity srcEntity, Entity dstEntity)
	{
		CopyComponent<Component...>(dst, srcEntity, dstEntity);
	}

	Ref<Scene> Scene::CopyTo(Ref<Scene> other)
	{
		ARC_PROFILE_SCOPE();

		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

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

		auto& srcRegistry = other->m_Registry;
		auto& dstRegistry = newScene->m_Registry;

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

		for (const auto& child : children)
		{
			if (Entity childEntity = GetEntity(child))
				DestroyEntity(childEntity);
		}

		m_EntityMap.erase(entity.GetUUID());
		m_Registry.destroy(entity);
	}

	Entity Scene::Duplicate(Entity entity)
	{
		ARC_PROFILE_SCOPE();

		eastl::string name = entity.GetComponent<TagComponent>().Tag;
		Entity duplicate = CreateEntity(name);
		CopyComponent(AllComponents{}, m_Registry, entity, duplicate);
		return duplicate;
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

		m_IsRunning = true;

		#pragma region Physics3D
		{
			ARC_PROFILE_CATEGORY("Physics 3D", Profile::Category::Physics);

			Physics3D::Init();
			auto view = m_Registry.view<TransformComponent, RigidbodyComponent>();
			for (auto e : view)
			{
				auto& body = view.get<RigidbodyComponent>(e);
				CreateRigidbody({ e, this }, body);
			}

			Physics3D::OptimizeBroadPhase();
		}
		#pragma endregion

		#pragma region Physics2D
		{
			ARC_PROFILE_CATEGORY("Physics 2D", Profile::Category::Physics);

			m_PhysicsWorld2D = new b2World({ 0.0f, -9.8f });
			m_ContactListener = new ContactListener(this);
			m_PhysicsWorld2D->SetContactListener(m_ContactListener);

			{
				auto view = m_Registry.view<TransformComponent, Rigidbody2DComponent>();
				for (auto e : view)
				{
					auto& body = view.get<Rigidbody2DComponent>(e);
					CreateRigidbody2D({ e, this }, body);
				}
			}

			{
				auto distanceJointView = m_Registry.view<Rigidbody2DComponent, DistanceJoint2DComponent>();
				for (auto e : distanceJointView)
				{
					auto [body, joint] = distanceJointView.get<Rigidbody2DComponent, DistanceJoint2DComponent>(e);
					Entity connectedBodyEntity = GetEntity(joint.ConnectedRigidbody);
					if (connectedBodyEntity && connectedBodyEntity.HasComponent<Rigidbody2DComponent>())
					{
						b2Body* body1 = (b2Body*)body.RuntimeBody;
						b2Body* body2 = (b2Body*)(connectedBodyEntity.GetComponent<Rigidbody2DComponent>().RuntimeBody);
						b2Vec2 body1Center = body1->GetWorldCenter();
						b2Vec2 body2Center = body2->GetWorldCenter();
						b2Vec2 anchor1Pos = { body1Center.x + joint.Anchor.x, body1Center.y + joint.Anchor.y };
						b2Vec2 anchor2Pos = { body2Center.x + joint.ConnectedAnchor.x, body2Center.y + joint.ConnectedAnchor.y };

						b2DistanceJointDef jointDef;
						jointDef.Initialize(body1, body2, anchor1Pos, anchor2Pos);
						jointDef.collideConnected = joint.EnableCollision;
						if (!joint.AutoDistance)
							jointDef.length = joint.Distance;
						jointDef.minLength = glm::min(jointDef.length, joint.MinDistance);
						jointDef.maxLength = jointDef.length + glm::max(joint.MaxDistanceBy, 0.0f);

						joint.RuntimeJoint = m_PhysicsWorld2D->CreateJoint(&jointDef);
					}
				}

				auto springJointView = m_Registry.view<Rigidbody2DComponent, SpringJoint2DComponent>();
				for (auto e : springJointView)
				{
					auto [body, joint] = springJointView.get<Rigidbody2DComponent, SpringJoint2DComponent>(e);
					Entity connectedBodyEntity = GetEntity(joint.ConnectedRigidbody);
					if (connectedBodyEntity && connectedBodyEntity.HasComponent<Rigidbody2DComponent>())
					{
						b2Body* body1 = (b2Body*)body.RuntimeBody;
						b2Body* body2 = (b2Body*)(connectedBodyEntity.GetComponent<Rigidbody2DComponent>().RuntimeBody);
						b2Vec2 body1Center = body1->GetWorldCenter();
						b2Vec2 body2Center = body2->GetWorldCenter();
						b2Vec2 anchor1Pos = { body1Center.x + joint.Anchor.x, body1Center.y + joint.Anchor.y };
						b2Vec2 anchor2Pos = { body2Center.x + joint.ConnectedAnchor.x, body2Center.y + joint.ConnectedAnchor.y };

						b2DistanceJointDef jointDef;
						jointDef.Initialize(body1, body2, anchor1Pos, anchor2Pos);
						jointDef.collideConnected = joint.EnableCollision;
						if (!joint.AutoDistance)
							jointDef.length = joint.Distance;
						jointDef.minLength = glm::min(jointDef.length, joint.MinDistance);
						jointDef.maxLength = jointDef.length + glm::max(joint.MaxDistanceBy, 0.0f);
						b2LinearStiffness(jointDef.stiffness, jointDef.damping, joint.Frequency, joint.DampingRatio, body1, body2);

						joint.RuntimeJoint = m_PhysicsWorld2D->CreateJoint(&jointDef);
					}
				}

				auto hingeJointView = m_Registry.view<Rigidbody2DComponent, HingeJoint2DComponent>();
				for (auto e : hingeJointView)
				{
					auto [body, joint] = hingeJointView.get<Rigidbody2DComponent, HingeJoint2DComponent>(e);
					Entity connectedBodyEntity = GetEntity(joint.ConnectedRigidbody);
					if (connectedBodyEntity && connectedBodyEntity.HasComponent<Rigidbody2DComponent>())
					{
						b2Body* body1 = (b2Body*)body.RuntimeBody;
						b2Body* body2 = (b2Body*)(connectedBodyEntity.GetComponent<Rigidbody2DComponent>().RuntimeBody);
						b2Vec2 body1Center = body1->GetWorldCenter();
						b2Vec2 anchorPos = { body1Center.x + joint.Anchor.x, body1Center.y + joint.Anchor.y };

						b2RevoluteJointDef jointDef;
						jointDef.Initialize(body1, body2, anchorPos);
						jointDef.collideConnected = joint.EnableCollision;
						jointDef.enableLimit = joint.UseLimits;
						jointDef.lowerAngle = glm::radians(joint.LowerAngle);
						jointDef.upperAngle = glm::radians(joint.UpperAngle);
						jointDef.enableMotor = joint.UseMotor;
						jointDef.motorSpeed = joint.MotorSpeed;
						jointDef.maxMotorTorque = joint.MaxMotorTorque;

						joint.RuntimeJoint = m_PhysicsWorld2D->CreateJoint(&jointDef);
					}
				}

				auto sliderJointView = m_Registry.view<Rigidbody2DComponent, SliderJoint2DComponent>();
				for (auto e : sliderJointView)
				{
					auto [body, joint] = sliderJointView.get<Rigidbody2DComponent, SliderJoint2DComponent>(e);
					Entity connectedBodyEntity = GetEntity(joint.ConnectedRigidbody);
					if (connectedBodyEntity && connectedBodyEntity.HasComponent<Rigidbody2DComponent>())
					{
						b2Body* body1 = (b2Body*)body.RuntimeBody;
						b2Body* body2 = (b2Body*)(connectedBodyEntity.GetComponent<Rigidbody2DComponent>().RuntimeBody);
						b2Vec2 body1Center = body1->GetWorldCenter();
						b2Vec2 anchorPos = { body1Center.x + joint.Anchor.x, body1Center.y + joint.Anchor.y };

						b2Vec2 worldAxis(1.0f, 0.0f);

						b2PrismaticJointDef jointDef;
						jointDef.Initialize(body1, body2, anchorPos, worldAxis);
						jointDef.collideConnected = joint.EnableCollision;
						jointDef.referenceAngle = glm::radians(joint.Angle);
						jointDef.enableLimit = joint.UseLimits;
						jointDef.lowerTranslation = joint.LowerTranslation;
						jointDef.upperTranslation = joint.UpperTranslation;
						jointDef.enableMotor = joint.UseMotor;
						jointDef.motorSpeed = joint.MotorSpeed;
						jointDef.maxMotorForce = joint.MaxMotorForce;

						joint.RuntimeJoint = m_PhysicsWorld2D->CreateJoint(&jointDef);
					}
				}

				auto wheelJointView = m_Registry.view<Rigidbody2DComponent, WheelJoint2DComponent>();
				for (auto e : wheelJointView)
				{
					auto [body, joint] = wheelJointView.get<Rigidbody2DComponent, WheelJoint2DComponent>(e);
					Entity connectedBodyEntity = GetEntity(joint.ConnectedRigidbody);
					if (connectedBodyEntity && connectedBodyEntity.HasComponent<Rigidbody2DComponent>())
					{
						b2Body* body1 = (b2Body*)body.RuntimeBody;
						b2Body* body2 = (b2Body*)(connectedBodyEntity.GetComponent<Rigidbody2DComponent>().RuntimeBody);
						b2Vec2 body1Center = body1->GetPosition();
						b2Vec2 anchorPos = { body1Center.x + joint.Anchor.x, body1Center.y + joint.Anchor.y };

						b2Vec2 axis(0.0f, 1.0f);

						float mass = body1->GetMass();
						float omega = 2.0f * b2_pi * joint.Frequency;

						b2WheelJointDef jd;
						jd.Initialize(body1, body2, anchorPos, axis);
						jd.collideConnected = joint.EnableCollision;
						jd.stiffness = mass * omega * omega;
						jd.damping = 2.0f * mass * joint.DampingRatio * omega;
						jd.enableMotor = joint.UseMotor;
						jd.motorSpeed = joint.MotorSpeed;
						jd.maxMotorTorque = joint.MaxMotorTorque;
						jd.enableLimit = joint.UseLimits;
						jd.lowerTranslation = joint.LowerTranslation;
						jd.upperTranslation = joint.UpperTranslation;

						joint.RuntimeJoint = m_PhysicsWorld2D->CreateJoint(&jd);
					}
				}
			}
		}
		#pragma endregion

		#pragma region Audio
		{
			ARC_PROFILE_CATEGORY("Audio", Profile::Category::Audio);

			auto listenerView = m_Registry.view<TransformComponent, AudioListenerComponent>();
			for (auto e : listenerView)
			{
				auto [tc, ac] = listenerView.get<TransformComponent, AudioListenerComponent>(e);
				ac.Listener = CreateRef<AudioListener>();
				if (ac.Active)
				{
					const glm::mat4 inverted = glm::inverse(Entity(e, this).GetWorldTransform());
					const glm::vec3 forward = normalize(glm::vec3(inverted[2]));
					ac.Listener->SetConfig(ac.Config);
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
					const glm::mat4 inverted = glm::inverse(Entity(e, this).GetWorldTransform());
					const glm::vec3 forward = normalize(glm::vec3(inverted[2]));
					ac.Source->SetConfig(ac.Config);
					ac.Source->SetPosition(tc.Translation);
					ac.Source->SetDirection(forward);
					if (ac.Config.PlayOnAwake)
						ac.Source->Play();
				}
			}
		}
		#pragma endregion

		#pragma region Scripting
		{
			ARC_PROFILE_CATEGORY("Script", Profile::Category::Script);

			auto scriptView = m_Registry.view<ScriptComponent>();
			for (auto e : scriptView)
			{
				const auto& sc = scriptView.get<ScriptComponent>(e);
				Entity entity = { e, this };

				for (auto& className : sc.Classes)
					ScriptEngine::CreateInstance(entity, className);

				for (auto& className : sc.Classes)
					ScriptEngine::GetInstance(entity, className)->InvokeOnCreate();
			}
		}
		#pragma endregion
	}

	void Scene::OnRuntimeStop()
	{
		ARC_PROFILE_SCOPE();

		m_IsRunning = false;

		#pragma region Scripting
		{
			ARC_PROFILE_CATEGORY("Script", Profile::Category::Script);

			auto scriptView = m_Registry.view<ScriptComponent>();
			for (auto e : scriptView)
			{
				ScriptComponent& script = scriptView.get<ScriptComponent>(e);
				Entity entity = { e, this };

				for (const auto& className : script.Classes)
				{
					ScriptEngine::GetInstance(entity, className)->InvokeOnDestroy();
					ScriptEngine::RemoveInstance(entity, className);
				}

				script.Classes.clear();
			}
		}
		#pragma endregion

		#pragma region Audio
		{
			ARC_PROFILE_CATEGORY("Audio", Profile::Category::Audio);

			auto view = m_Registry.view<AudioSourceComponent>();
			for (auto e : view)
			{
				const auto& ac = view.get<AudioSourceComponent>(e);
				if (ac.Source)
					ac.Source->Stop();
			}
		}
		#pragma endregion

		#pragma region Physics2D
		{
			ARC_PROFILE_CATEGORY("Physics 2D", Profile::Category::Physics);

			m_FixtureMap.clear();
			delete m_ContactListener;
			delete m_PhysicsWorld2D;
			m_ContactListener = nullptr;
			m_PhysicsWorld2D = nullptr;
		}
		#pragma endregion

		#pragma region Physics3D
		{
			ARC_PROFILE_CATEGORY("Physics 3D", Profile::Category::Physics);

			auto* bodyInterface = Physics3D::GetBodyInterface();
			auto view = m_Registry.view<RigidbodyComponent>();
			for (auto e : view)
			{
				const auto& rb = view.get<RigidbodyComponent>(e);
				if (rb.RuntimeBody)
				{
					JPH::Body* body = (JPH::Body*)rb.RuntimeBody;
					bodyInterface->RemoveBody(body->GetID());
					bodyInterface->DestroyBody(body->GetID());
				}
			}

			Physics3D::Shutdown();
		}
		#pragma endregion
	}

	void Scene::OnUpdateEditor([[maybe_unused]] Timestep ts, const Ref<RenderGraphData>& renderGraphData, const EditorCamera& camera)
	{
		ARC_PROFILE_SCOPE();
		
		CameraData cameraData = {};
		cameraData.View = camera.GetViewMatrix();
		cameraData.Projection = camera.GetProjectionMatrix();
		cameraData.ViewProjection = cameraData.Projection * cameraData.View;
		cameraData.Position = camera.GetPosition();

		OnRender(renderGraphData, cameraData);
	}

	void Scene::OnUpdateRuntime([[maybe_unused]] Timestep ts, const Ref<RenderGraphData>& renderGraphData, const EditorCamera* overrideCamera)
	{
		ARC_PROFILE_SCOPE();

		#pragma region Physics3D
		{
			ARC_PROFILE_CATEGORY("Physics 3D", Profile::Category::Physics);

			auto* bodyInterface = Physics3D::GetBodyInterface();
			auto view = m_Registry.view<TransformComponent, RigidbodyComponent>();
			for (auto e : view)
			{
				auto [tc, body] = view.get<TransformComponent, RigidbodyComponent>(e);
				if (body.RuntimeBody)
				{
					JPH::Body* rb = (JPH::Body*)body.RuntimeBody;
					JPH::EActivation activation = bodyInterface->IsActive(rb->GetID()) ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;
					glm::quat rotation = glm::quat(tc.Rotation);
					bodyInterface->SetPositionAndRotation(rb->GetID(), { tc.Translation.x, tc.Translation.y, tc.Translation.z }, { rotation.x, rotation.y, rotation.z, rotation.w }, activation);
				}
			}
		}
		#pragma endregion

		#pragma region Physics2D
		{
			ARC_PROFILE_CATEGORY("Physics 2D", Profile::Category::Physics);

			auto view = m_Registry.view<TransformComponent, Rigidbody2DComponent>();
			for (auto e : view)
			{
				auto [tc, body] = view.get<TransformComponent, Rigidbody2DComponent>(e);
				b2Body* rb = (b2Body*)body.RuntimeBody;
				rb->SetTransform(b2Vec2(tc.Translation.x, tc.Translation.y), tc.Rotation.z);
			}
		}
		#pragma endregion

		#pragma region Scripting
		{
			ARC_PROFILE_CATEGORY("Script", Profile::Category::Script);

			auto scriptView = m_Registry.view<ScriptComponent>();
			for (auto e : scriptView)
			{
				const ScriptComponent& script = scriptView.get<ScriptComponent>(e);
				Entity entity = { e, this };

				for (const auto& className : script.Classes)
					ScriptEngine::GetInstance(entity, className)->InvokeOnUpdate(ts);
			}
		}
		#pragma endregion
		
		constexpr float physicsStepRate = 60.0f;
		constexpr float physicsTs = 1.0f / physicsStepRate;

		#pragma region Physics3D
		{
			ARC_PROFILE_CATEGORY("Physics 3D", Profile::Category::Physics);

			Physics3D::Step(physicsTs);

			auto view = m_Registry.view<TransformComponent, RigidbodyComponent>();
			for (auto e : view)
			{
				auto [tc, rb] = view.get<TransformComponent, RigidbodyComponent>(e);
				if (rb.RuntimeBody)
				{
					JPH::Body* body = (JPH::Body*)rb.RuntimeBody;
					JPH::Vec3 position = body->GetPosition();
					JPH::Vec3 rotation = body->GetRotation().GetEulerAngles();
					tc.Translation = glm::vec3(position.GetX(), position.GetY(), position.GetZ());
					tc.Rotation = glm::vec3(rotation.GetX(), rotation.GetY(), rotation.GetZ());
				}
			}
		}
		#pragma endregion

		#pragma region Physics2D
		{
			ARC_PROFILE_CATEGORY("Physics 2D", Profile::Category::Physics);
			
			m_ContactListener->OnUpdate(physicsTs);
			m_PhysicsWorld2D->Step(physicsTs, VelocityIterations, PositionIterations);

			auto view = m_Registry.view<TransformComponent, Rigidbody2DComponent>();
			for (auto e : view)
			{
				auto [transform, body] = view.get<TransformComponent, Rigidbody2DComponent>(e);
				const b2Body* rb = (b2Body*)body.RuntimeBody;
				b2Vec2 position = rb->GetPosition();
				transform.Translation.x = position.x;
				transform.Translation.y = position.y;
				transform.Rotation.z = rb->GetAngle();
			}

			auto distanceJointView = m_Registry.view<DistanceJoint2DComponent>();
			for (auto e : distanceJointView)
			{
				auto& joint = distanceJointView.get<DistanceJoint2DComponent>(e);
				if (joint.RuntimeJoint)
				{
					b2Joint* j = (b2Joint*)joint.RuntimeJoint;
					
					if (j->GetReactionForce(physicsStepRate).LengthSquared() > joint.BreakForce * joint.BreakForce)
					{
						m_PhysicsWorld2D->DestroyJoint(j);
						joint.RuntimeJoint = nullptr;
					}
				}
			}

			auto springJointView = m_Registry.view<SpringJoint2DComponent>();
			for (auto e : springJointView)
			{
				auto& joint = springJointView.get<SpringJoint2DComponent>(e);
				if (joint.RuntimeJoint)
				{
					b2Joint* j = (b2Joint*)joint.RuntimeJoint;

					if (j->GetReactionForce(physicsStepRate).LengthSquared() > joint.BreakForce * joint.BreakForce)
					{
						m_PhysicsWorld2D->DestroyJoint(j);
						joint.RuntimeJoint = nullptr;
					}
				}
			}

			auto hingeJointView = m_Registry.view<HingeJoint2DComponent>();
			for (auto e : hingeJointView)
			{
				auto& joint = hingeJointView.get<HingeJoint2DComponent>(e);
				if (joint.RuntimeJoint)
				{
					b2Joint* j = (b2Joint*)joint.RuntimeJoint;

					if (j->GetReactionForce(physicsStepRate).LengthSquared() > joint.BreakForce * joint.BreakForce
						|| j->GetReactionTorque(physicsStepRate) > joint.BreakTorque)
					{
						m_PhysicsWorld2D->DestroyJoint(j);
						joint.RuntimeJoint = nullptr;
					}
				}
			}

			auto sliderJointView = m_Registry.view<SliderJoint2DComponent>();
			for (auto e : sliderJointView)
			{
				auto& joint = sliderJointView.get<SliderJoint2DComponent>(e);
				if (joint.RuntimeJoint)
				{
					b2Joint* j = (b2Joint*)joint.RuntimeJoint;

					if (j->GetReactionForce(physicsStepRate).LengthSquared() > joint.BreakForce * joint.BreakForce
						|| j->GetReactionTorque(physicsStepRate) > joint.BreakTorque)
					{
						m_PhysicsWorld2D->DestroyJoint(j);
						joint.RuntimeJoint = nullptr;
					}
				}
			}

			auto wheelJointView = m_Registry.view<WheelJoint2DComponent>();
			for (auto e : wheelJointView)
			{
				auto& joint = wheelJointView.get<WheelJoint2DComponent>(e);
				if (joint.RuntimeJoint)
				{
					b2Joint* j = (b2Joint*)joint.RuntimeJoint;

					if (j->GetReactionForce(physicsStepRate).LengthSquared() > joint.BreakForce * joint.BreakForce
						|| j->GetReactionTorque(physicsStepRate) > joint.BreakTorque)
					{
						m_PhysicsWorld2D->DestroyJoint(j);
						joint.RuntimeJoint = nullptr;
					}
				}
			}
		}
		#pragma endregion

		#pragma region Audio
		{
			ARC_PROFILE_CATEGORY("Audio", Profile::Category::Audio);

			auto listenerView = m_Registry.view<TransformComponent, AudioListenerComponent>();
			for (auto e : listenerView)
			{
				auto [tc, ac] = listenerView.get<TransformComponent, AudioListenerComponent>(e);
				if (ac.Active)
				{
					const glm::mat4 inverted = glm::inverse(Entity(e, this).GetWorldTransform());
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
		#pragma endregion

		#pragma region Rendering
		CameraData cameraData = {};
		{
			ARC_PROFILE_CATEGORY("Camera", Profile::Category::Camera);

			Entity cameraEntity = GetPrimaryCameraEntity();
			if (!overrideCamera)
			{
				if (cameraEntity)
				{
					cameraData.View = glm::inverse(cameraEntity.GetWorldTransform());
					cameraData.Projection = cameraEntity.GetComponent<CameraComponent>().Camera.GetProjection();
					cameraData.ViewProjection = cameraData.Projection * cameraData.View;
					cameraData.Position = cameraEntity.GetTransform().Translation;
				}
			}
			else
			{
				cameraData.View = overrideCamera->GetViewMatrix();
				cameraData.Projection = overrideCamera->GetProjectionMatrix();
				cameraData.ViewProjection = cameraData.Projection * cameraData.View;
				cameraData.Position = overrideCamera->GetPosition();
			}
		}

		OnRender(renderGraphData, cameraData);
		#pragma endregion
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
		ARC_PROFILE_CATEGORY("Camera", Profile::Category::Camera);

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity(entity, this);
		}
		return {};
	}

	void Scene::OnRender(const Ref<RenderGraphData>& renderGraphData, const CameraData& cameraData)
	{
		ARC_PROFILE_CATEGORY("Rendering", Profile::Category::Rendering);

		eastl::vector<Entity> lights;
		{
			ARC_PROFILE_SCOPE("Prepare Light Data");

			auto view = m_Registry.view<LightComponent>();
			lights.reserve(view.size());
			for (auto entity : view)
				lights.emplace_back(Entity(entity, this));
		}
		Entity skylight = {};
		{
			ARC_PROFILE_SCOPE("PrepareSkylightData");

			auto view = m_Registry.view<SkyLightComponent>();
			if (!view.empty())
				skylight = Entity(*view.begin(), this);
		}

		Renderer3D::BeginScene(cameraData, skylight, eastl::move(lights));
		// Meshes
		{
			ARC_PROFILE_SCOPE("Submit Mesh Data");

			auto view = m_Registry.view<MeshComponent>();
			Renderer3D::ReserveMeshes(view.size());
			for (auto entity : view)
			{
				const auto& meshComponent = view.get<MeshComponent>(entity);
				if (meshComponent.MeshGeometry && meshComponent.MeshGeometry->GetSubmeshCount() != 0)
				{
					ARC_CORE_ASSERT(meshComponent.MeshGeometry->GetSubmeshCount() > meshComponent.SubmeshIndex, "Trying to access submesh index that does not exist!");
					Renderer3D::SubmitMesh(Entity(entity, this).GetWorldTransform(), meshComponent.MeshGeometry->GetSubmesh(meshComponent.SubmeshIndex), meshComponent.CullMode);
				}
			}
		}
		Renderer3D::EndScene(renderGraphData);

		Renderer2D::BeginScene(cameraData.ViewProjection);
		{
			ARC_PROFILE_SCOPE("Submit 2D Data");

			auto view = m_Registry.view<SpriteRendererComponent>();
			for (auto entity : view)
			{
				const auto& sprite = view.get<SpriteRendererComponent>(entity);
				Renderer2D::DrawQuad(Entity(entity, this).GetWorldTransform(), sprite.Texture, sprite.Color, sprite.TilingFactor);
			}
		}
		Renderer2D::EndScene(renderGraphData);
	}

	void Scene::CreateRigidbody(Entity entity, RigidbodyComponent& component)
	{
		if (!m_IsRunning)
			return;

		auto* bodyInterface = Physics3D::GetBodyInterface();
		if (component.RuntimeBody)
		{
			bodyInterface->DestroyBody(((JPH::Body*)component.RuntimeBody)->GetID());
			component.RuntimeBody = nullptr;
		}

		TransformComponent tc = entity.GetComponent<TransformComponent>();

		JPH::CompoundShapeSettings* compoundShapeSettings;
		if (component.Type == RigidbodyComponent::BodyType::Static)
			compoundShapeSettings = new JPH::StaticCompoundShapeSettings();
		else
			compoundShapeSettings = new JPH::StaticCompoundShapeSettings();

		float maxScaleComponent = glm::max(glm::max(tc.Scale.x, tc.Scale.y), tc.Scale.z);

		const char* entityName = entity.GetComponent<TagComponent>().Tag.c_str();

		if (entity.HasComponent<BoxColliderComponent>())
		{
			auto& bc = entity.GetComponent<BoxColliderComponent>();
			PhysicsMaterial3D* mat = new PhysicsMaterial3D(entityName, JPH::ColorArg(255, 0, 0), bc.Friction, bc.Restitution);
			
			glm::vec3 scale = bc.Size * tc.Scale * 2.0f;
			JPH::BoxShapeSettings shapeSettings({ glm::abs(scale.x), glm::abs(scale.y), glm::abs(scale.z) }, 0.05f, mat);
			shapeSettings.SetDensity(glm::max(0.001f, bc.Density));

			compoundShapeSettings->AddShape({ bc.Offset.x, bc.Offset.y, bc.Offset.z }, JPH::Quat::sIdentity(), shapeSettings.Create().Get());
		}

		if (entity.HasComponent<SphereColliderComponent>())
		{
			auto& sc = entity.GetComponent<SphereColliderComponent>();
			PhysicsMaterial3D* mat = new PhysicsMaterial3D(entityName, JPH::ColorArg(255, 0, 0), sc.Friction, sc.Restitution);

			float radius = 2.0f * sc.Radius * maxScaleComponent;
			JPH::SphereShapeSettings shapeSettings(glm::max(0.01f, radius), mat);
			shapeSettings.SetDensity(glm::max(0.001f, sc.Density));

			compoundShapeSettings->AddShape({ sc.Offset.x, sc.Offset.y, sc.Offset.z }, JPH::Quat::sIdentity(), shapeSettings.Create().Get());
		}

		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			auto& cc = entity.GetComponent<CapsuleColliderComponent>();
			PhysicsMaterial3D* mat = new PhysicsMaterial3D(entityName, JPH::ColorArg(255, 0, 0), cc.Friction, cc.Restitution);

			float radius = 2.0f * cc.Radius * maxScaleComponent;
			JPH::CapsuleShapeSettings shapeSettings(glm::max(0.01f, cc.Height) * 0.5f, glm::max(0.01f, radius), mat);
			shapeSettings.SetDensity(glm::max(0.001f, cc.Density));

			compoundShapeSettings->AddShape({ cc.Offset.x, cc.Offset.y, cc.Offset.z }, JPH::Quat::sIdentity(), shapeSettings.Create().Get());
		}

		if (entity.HasComponent<TaperedCapsuleColliderComponent>())
		{
			auto& tcc = entity.GetComponent<TaperedCapsuleColliderComponent>();
			PhysicsMaterial3D* mat = new PhysicsMaterial3D(entityName, JPH::ColorArg(255, 0, 0), tcc.Friction, tcc.Restitution);

			float topRadius = 2.0f * tcc.TopRadius * maxScaleComponent;
			float bottomRadius = 2.0f * tcc.BottomRadius * maxScaleComponent;
			JPH::TaperedCapsuleShapeSettings shapeSettings(glm::max(0.01f, tcc.Height) * 0.5f, glm::max(0.01f, topRadius), glm::max(0.01f, bottomRadius), mat);
			shapeSettings.SetDensity(glm::max(0.001f, tcc.Density));

			compoundShapeSettings->AddShape({ tcc.Offset.x, tcc.Offset.y, tcc.Offset.z }, JPH::Quat::sIdentity(), shapeSettings.Create().Get());
		}

		if (entity.HasComponent<CylinderColliderComponent>())
		{
			auto& cc = entity.GetComponent<CylinderColliderComponent>();
			PhysicsMaterial3D* mat = new PhysicsMaterial3D(entityName, JPH::ColorArg(255, 0, 0), cc.Friction, cc.Restitution);

			float radius = 2.0f * cc.Radius * maxScaleComponent;
			JPH::CylinderShapeSettings shapeSettings(glm::max(0.01f, cc.Height) * 0.5f, glm::max(0.01f, radius), 0.05f, mat);
			shapeSettings.SetDensity(glm::max(0.001f, cc.Density));

			compoundShapeSettings->AddShape({ cc.Offset.x, cc.Offset.y, cc.Offset.z }, JPH::Quat::sIdentity(), shapeSettings.Create().Get());
		}

		// Body
		glm::quat rotation = glm::quat(tc.Rotation);
		JPH::BodyCreationSettings bodySettings(compoundShapeSettings->Create().Get(), {tc.Translation.x, tc.Translation.y, tc.Translation.z}, {rotation.x, rotation.y, rotation.z, rotation.w}, (JPH::EMotionType)component.Type, Physics3D::Layers::MOVING);

		delete compoundShapeSettings;

		if (!component.AutoMass)
		{
			JPH::MassProperties massProperties;
			massProperties.mMass = glm::max(0.01f, component.Mass);
			bodySettings.mMassPropertiesOverride = massProperties;
			bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
		}
		bodySettings.mAllowSleeping = component.AllowSleep;
		bodySettings.mLinearDamping = glm::max(0.0f, component.LinearDrag);
		bodySettings.mAngularDamping = glm::max(0.0f, component.AngularDrag);
		bodySettings.mMotionQuality = component.Continuous ? JPH::EMotionQuality::LinearCast : JPH::EMotionQuality::Discrete;
		bodySettings.mGravityFactor = component.GravityScale;

		bodySettings.mIsSensor = component.IsSensor;

		JPH::Body* body = bodyInterface->CreateBody(bodySettings);

		JPH::EActivation activation = component.Awake && component.Type != RigidbodyComponent::BodyType::Static ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;
		bodyInterface->AddBody(body->GetID(), activation);

		component.RuntimeBody = body;
	}

	void Scene::CreateRigidbody2D(Entity entity, Rigidbody2DComponent& body)
	{
		ARC_PROFILE_SCOPE();

		if (m_PhysicsWorld2D)
		{
			const TransformComponent& transform = entity.GetTransform();

			b2BodyDef def;
			def.type = (b2BodyType)body.Type;
			def.linearDamping = glm::max(body.LinearDrag, 0.0f);
			def.angularDamping = glm::max(body.AngularDrag, 0.0f);
			def.allowSleep = body.AllowSleep;
			def.awake = body.Awake;
			def.fixedRotation = body.FreezeRotation;
			def.bullet = body.Continuous;
			def.gravityScale = body.GravityScale;

			def.position.Set(transform.Translation.x, transform.Translation.y);
			def.angle = transform.Rotation.z;

			b2Body* rb = m_PhysicsWorld2D->CreateBody(&def);
			body.RuntimeBody = rb;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
				CreateBoxCollider2D(entity, bc2d);
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
				CreateCircleCollider2D(entity, cc2d);
			}

			if (!body.AutoMass && body.Mass > 0.01f)
			{
				b2MassData massData = rb->GetMassData();
				massData.mass = body.Mass;
				rb->SetMassData(&massData);
			}
		}
	}

	void Scene::CreateBoxCollider2D(Entity entity, BoxCollider2DComponent& bc2d)
	{
		ARC_PROFILE_SCOPE();

		if (m_PhysicsWorld2D && entity.HasComponent<Rigidbody2DComponent>())
		{
			const TransformComponent& transform = entity.GetTransform();
			b2Body* rb = (b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody;

			b2PolygonShape boxShape;
			boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y, { bc2d.Offset.x, bc2d.Offset.y }, 0.0f);

			b2FixtureDef fixtureDef;
			fixtureDef.shape = &boxShape;
			fixtureDef.isSensor = bc2d.IsSensor;
			fixtureDef.density = bc2d.Density;
			fixtureDef.friction = bc2d.Friction;
			fixtureDef.restitution = bc2d.Restitution;
			fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;

			b2Fixture* fixture = rb->CreateFixture(&fixtureDef);
			bc2d.RuntimeFixture = fixture;
			m_FixtureMap[fixture] = entity;
		}
	}

	void Scene::CreateCircleCollider2D(Entity entity, CircleCollider2DComponent& cc2d)
	{
		ARC_PROFILE_SCOPE();

		if (m_PhysicsWorld2D && entity.HasComponent<Rigidbody2DComponent>())
		{
			const TransformComponent& transform = entity.GetTransform();
			b2Body* rb = (b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody;

			b2CircleShape circleShape;
			circleShape.m_radius = cc2d.Radius * glm::max(transform.Scale.x, transform.Scale.y);
			circleShape.m_p = { cc2d.Offset.x, cc2d.Offset.y };

			b2FixtureDef fixtureDef;
			fixtureDef.shape = &circleShape;
			fixtureDef.isSensor = cc2d.IsSensor;
			fixtureDef.density = cc2d.Density;
			fixtureDef.friction = cc2d.Friction;
			fixtureDef.restitution = cc2d.Restitution;
			fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;

			b2Fixture* fixture = rb->CreateFixture(&fixtureDef);
			cc2d.RuntimeFixture = fixture;
			m_FixtureMap[fixture] = entity;
		}
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false);
	}

	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{
		/* On IDComponent added */
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
		/* On TransformComponent added */
	}

	template<>
	void Scene::OnComponentAdded<RelationshipComponent>(Entity entity, RelationshipComponent& component)
	{
		/* On RelationshipComponent added */
	}

	template<>
	void Scene::OnComponentAdded<PrefabComponent>(Entity entity, PrefabComponent& component)
	{
		/* On PrefabComponent added */
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		/* On CameraComponent added */
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
		/* On SpriteRendererComponent added */
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
		/* On TagComponent added */
	}

	template<>
	void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component)
	{
		/* On MeshComponent added */
	}

	template<>
	void Scene::OnComponentAdded<SkyLightComponent>(Entity entity, SkyLightComponent& component)
	{
		/* On SkyLightComponent added */
	}

	template<>
	void Scene::OnComponentAdded<LightComponent>(Entity entity, LightComponent& component)
	{
		/* On LightComponent added */
	}

	template<>
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& body)
	{
		/* On Rigidbody2DComponent added */
		CreateRigidbody2D(entity, body);
	}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& bc2d)
	{
		/* On BoxCollider2DComponent added */
		CreateBoxCollider2D(entity, bc2d);
	}
	
	template<>
	void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& cc2d)
	{
		/* On CircleCollider2DComponent added */
		CreateCircleCollider2D(entity, cc2d);
	}

	template<>
	void Scene::OnComponentAdded<DistanceJoint2DComponent>(Entity entity, DistanceJoint2DComponent& component)
	{
		/* On DistanceJoint2DComponent added */
	}

	template<>
	void Scene::OnComponentAdded<SpringJoint2DComponent>(Entity entity, SpringJoint2DComponent& component)
	{
		/* On SpringJoint2DComponent added */
	}

	template<>
	void Scene::OnComponentAdded<HingeJoint2DComponent>(Entity entity, HingeJoint2DComponent& component)
	{
		/* On HingeJoint2DComponent added */
	}

	template<>
	void Scene::OnComponentAdded<SliderJoint2DComponent>(Entity entity, SliderJoint2DComponent& component)
	{
		/* On SliderJoint2DComponent added */
	}

	template<>
	void Scene::OnComponentAdded<WheelJoint2DComponent>(Entity entity, WheelJoint2DComponent& component)
	{
		/* On WheelJoint2DComponent added */
	}

	template<>
	void Scene::OnComponentAdded<BuoyancyEffector2DComponent>(Entity entity, BuoyancyEffector2DComponent& component)
	{
		/* On BuoyancyEffector2DComponent added */
	}

	template<>
	void Scene::OnComponentAdded<RigidbodyComponent>(Entity entity, RigidbodyComponent& body)
	{
		/* On RigidbodyComponent added */
		if (IsRunning())
			CreateRigidbody(entity, body);
	}

	template<>
	void Scene::OnComponentAdded<BoxColliderComponent>(Entity entity, BoxColliderComponent& bc)
	{
		/* On BoxColliderComponent added */
		if (entity.HasComponent<RigidbodyComponent>())
			CreateRigidbody(entity, entity.GetComponent<RigidbodyComponent>());
	}

	template<>
	void Scene::OnComponentAdded<SphereColliderComponent>(Entity entity, SphereColliderComponent& sc)
	{
		/* On SphereColliderComponent added */
		if (entity.HasComponent<RigidbodyComponent>())
			CreateRigidbody(entity, entity.GetComponent<RigidbodyComponent>());
	}

	template<>
	void Scene::OnComponentAdded<CapsuleColliderComponent>(Entity entity, CapsuleColliderComponent& sc)
	{
		/* On SphereColliderComponent added */
		if (entity.HasComponent<RigidbodyComponent>())
			CreateRigidbody(entity, entity.GetComponent<RigidbodyComponent>());
	}

	template<>
	void Scene::OnComponentAdded<TaperedCapsuleColliderComponent>(Entity entity, TaperedCapsuleColliderComponent& sc)
	{
		/* On SphereColliderComponent added */
		if (entity.HasComponent<RigidbodyComponent>())
			CreateRigidbody(entity, entity.GetComponent<RigidbodyComponent>());
	}

	template<>
	void Scene::OnComponentAdded<CylinderColliderComponent>(Entity entity, CylinderColliderComponent& sc)
	{
		/* On SphereColliderComponent added */
		if (entity.HasComponent<RigidbodyComponent>())
			CreateRigidbody(entity, entity.GetComponent<RigidbodyComponent>());
	}

	template<>
	void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component)
	{
		/* On ScriptComponent added */
	}

	template<>
	void Scene::OnComponentAdded<AudioSourceComponent>(Entity entity, AudioSourceComponent& component)
	{
		/* On AudioSourceComponent added */
	}

	template<>
	void Scene::OnComponentAdded<AudioListenerComponent>(Entity entity, AudioListenerComponent& component)
	{
		/* On AudioListenerComponent added */
	}
}
