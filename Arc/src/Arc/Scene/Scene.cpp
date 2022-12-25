#include "arcpch.h"
#include "Arc/Scene/Scene.h"

#include "Arc/Physics/Physics3D.h"
#include "Arc/Physics/PhysicsUtils.h"
#include "Arc/Renderer/EditorCamera.h"
#include "Arc/Renderer/Renderer2D.h"
#include "Arc/Renderer/Renderer3D.h"
#include "Arc/Renderer/RenderGraphData.h"
#include "Arc/Scene/Components.h"
#include "Arc/Scene/Entity.h"
#include "Arc/Scripting/ScriptEngine.h"

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>
#include <box2d/box2d.h>

// Jolt includes
#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include "Arc/Physics/PhysicsMaterial3D.h"

namespace ArcEngine
{
	std::map<EntityLayer, EntityLayerData> Scene::LayerCollisionMask =
	{
		{ BIT(0), { "Static",		static_cast<uint16_t>(0xFFFF), 0 } },
		{ BIT(1), { "Default",		static_cast<uint16_t>(0xFFFF), 1 } },
		{ BIT(2), { "Player",		static_cast<uint16_t>(0xFFFF), 2 } },
		{ BIT(3), { "Sensor",		static_cast<uint16_t>(0xFFFF), 3 } },
	};

	#pragma region Physics2DListeners

	class Physics2DContactListener : public b2ContactListener
	{
	public:
		explicit Physics2DContactListener(Scene* scene)
			: m_Scene(scene)
		{
			ARC_PROFILE_SCOPE()

		}

		~Physics2DContactListener() override
		{
			ARC_PROFILE_SCOPE()

			m_BuoyancyFixtures.clear();
		}

		Physics2DContactListener(const Physics2DContactListener& other) = delete;
		Physics2DContactListener(Physics2DContactListener&& other) = delete;
		Physics2DContactListener& operator=(const Physics2DContactListener& other) = delete;
		Physics2DContactListener& operator=(Physics2DContactListener&& other) = delete;

		void BeginContact(b2Contact* contact) override
		{
			ARC_PROFILE_SCOPE()

			b2Fixture* a = contact->GetFixtureA();
			b2Fixture* b = contact->GetFixtureB();

			bool aSensor = a->IsSensor();
			bool bSensor = b->IsSensor();
			Entity e1 = { static_cast<entt::entity>(static_cast<uint32_t>(a->GetUserData().pointer)), m_Scene };
			Entity e2 = { static_cast<entt::entity>(static_cast<uint32_t>(b->GetUserData().pointer)), m_Scene };

			if (e1.HasComponent<BuoyancyEffector2DComponent>() && aSensor
				&& !e2.HasComponent<BuoyancyEffector2DComponent>() && b->GetBody()->GetType() == b2_dynamicBody)
			{
				m_BuoyancyFixtures.insert(std::make_pair(a, b));
			}
			else if (e2.HasComponent<BuoyancyEffector2DComponent>() && bSensor
				&& !e1.HasComponent<BuoyancyEffector2DComponent>() && a->GetBody()->GetType() == b2_dynamicBody)
			{
				m_BuoyancyFixtures.insert(std::make_pair(b, a));
			}

			b2WorldManifold worldManifold{};
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

		void EndContact(b2Contact* contact) override
		{
			ARC_PROFILE_SCOPE()

			b2Fixture* a = contact->GetFixtureA();
			b2Fixture* b = contact->GetFixtureB();
			bool aSensor = a->IsSensor();
			bool bSensor = b->IsSensor();
			Entity e1 = { static_cast<entt::entity>(static_cast<uint32_t>(a->GetUserData().pointer)), m_Scene };
			Entity e2 = { static_cast<entt::entity>(static_cast<uint32_t>(b->GetUserData().pointer)), m_Scene };

			if (e1.HasComponent<BuoyancyEffector2DComponent>() && aSensor
				&& !e2.HasComponent<BuoyancyEffector2DComponent>() && b->GetBody()->GetType() == b2_dynamicBody)
			{
				m_BuoyancyFixtures.erase(std::make_pair(a, b));
			}
			else if (e2.HasComponent<BuoyancyEffector2DComponent>() && bSensor
				&& !e1.HasComponent<BuoyancyEffector2DComponent>() && a->GetBody()->GetType() == b2_dynamicBody)
			{
				m_BuoyancyFixtures.erase(std::make_pair(b, a));
			}

			b2WorldManifold worldManifold{};
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

		void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override
		{
			ARC_PROFILE_SCOPE()

			/* Handle pre solve */
		}

		void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override
		{
			ARC_PROFILE_SCOPE()

			/* Handle post solve */
		}

		void OnUpdate([[maybe_unused]] Timestep ts)
		{
			ARC_PROFILE_SCOPE()

			auto it = m_BuoyancyFixtures.begin();
			auto end = m_BuoyancyFixtures.end();
			while (it != end)
			{
				b2Fixture* fluid = it->first;
				b2Fixture* fixture = it->second;

				Entity fluidEntity = { static_cast<entt::entity>(static_cast<uint32_t>(fluid->GetUserData().pointer)), m_Scene };
				const auto& buoyancyComponent2D = fluidEntity.GetComponent<BuoyancyEffector2DComponent>();
				b2Vec2 gravity = { m_Scene->Gravity.x, m_Scene->Gravity.y };
				PhysicsUtils::HandleBuoyancy(fluid, fixture, gravity, buoyancyComponent2D.FlipGravity, buoyancyComponent2D.Density, buoyancyComponent2D.DragMultiplier, buoyancyComponent2D.FlowMagnitude, buoyancyComponent2D.FlowAngle);
				++it;
			}
		}

	private:
		Scene* m_Scene;

		std::set<std::pair<b2Fixture*, b2Fixture*>> m_BuoyancyFixtures;
	};
	
	#pragma endregion

	#pragma region Physics3DListeners

	class Physics3DContactListener : public JPH::ContactListener
	{
	private:
		static void	GetFrictionAndRestitution(const JPH::Body& inBody, const JPH::SubShapeID& inSubShapeID, float& outFriction, float& outRestitution)
		{
			ARC_PROFILE_SCOPE()

			// Get the material that corresponds to the sub shape ID
			const JPH::PhysicsMaterial* material = inBody.GetShape()->GetMaterial(inSubShapeID);
			if (material == JPH::PhysicsMaterial::sDefault)
			{
				outFriction = inBody.GetFriction();
				outRestitution = inBody.GetRestitution();
			}
			else
			{
				const auto* phyMaterial = static_cast<const PhysicsMaterial3D*>(material);
				outFriction = phyMaterial->Friction;
				outRestitution = phyMaterial->Restitution;
			}
		}

		static void	OverrideContactSettings(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
		{
			ARC_PROFILE_SCOPE()

			// Get the custom friction and restitution for both bodies
			float friction1, friction2, restitution1, restitution2;
			GetFrictionAndRestitution(inBody1, inManifold.mSubShapeID1, friction1, restitution1);
			GetFrictionAndRestitution(inBody2, inManifold.mSubShapeID2, friction2, restitution2);

			// Use the default formulas for combining friction and restitution
			ioSettings.mCombinedFriction = JPH::sqrt(friction1 * friction2);
			ioSettings.mCombinedRestitution = JPH::max(restitution1, restitution2);
		}

	public:
		JPH::ValidateResult OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::CollideShapeResult& inCollisionResult) override
		{
			ARC_PROFILE_SCOPE()

			return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
		}

		void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
		{
			ARC_PROFILE_SCOPE()

			OverrideContactSettings(inBody1, inBody2, inManifold, ioSettings);
		}

		void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
		{
			ARC_PROFILE_SCOPE()

			OverrideContactSettings(inBody1, inBody2, inManifold, ioSettings);
		}

		void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
		{
			ARC_PROFILE_SCOPE()

			/* On Collision Exit */
		}
	};

	class Physics3DBodyActivationListener : public JPH::BodyActivationListener
	{
	public:
		void OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
		{
			ARC_PROFILE_SCOPE()

			/* Body Activated */
		}

		void OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
		{
			ARC_PROFILE_SCOPE()

			/* Body Deactivated */
		}
	};

	#pragma endregion

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, std::unordered_map<UUID, entt::entity> enttMap)
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
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, std::unordered_map<UUID, entt::entity> enttMap)
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

	Ref<Scene> Scene::CopyTo(const Ref<Scene>& other)
	{
		ARC_PROFILE_SCOPE()

		Ref<Scene> newScene = CreateRef<Scene>();
		newScene->VelocityIterations = other->VelocityIterations;
		newScene->PositionIterations = other->PositionIterations;

		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		auto view = other->m_Registry.group<IDComponent, TagComponent>();
		for (auto it = view.rbegin(); it != view.rend(); ++it)
		{
			auto [id, tag] = view.get<IDComponent, TagComponent>(*it);
			Entity newEntity = newScene->CreateEntityWithUUID(id.ID, tag.Tag);
			newEntity.GetComponent<TagComponent>().Enabled = tag.Enabled;
			newEntity.GetComponent<TagComponent>().Layer = tag.Layer;
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

	Entity Scene::CreateEntity(const std::string& name)
	{
		ARC_PROFILE_SCOPE()

		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		ARC_PROFILE_SCOPE()

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
		ARC_PROFILE_SCOPE()

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
		ARC_PROFILE_SCOPE()

		std::string name = entity.GetComponent<TagComponent>().Tag;
		Entity duplicate = CreateEntity(name);
		CopyComponent(AllComponents{}, m_Registry, entity, duplicate);
		return duplicate;
	}

	bool Scene::HasEntity(UUID uuid) const
	{
		ARC_PROFILE_SCOPE()

		return m_EntityMap.contains(uuid);
	}

	Entity Scene::GetEntity(UUID uuid)
	{
		ARC_PROFILE_SCOPE()

		const auto& it = m_EntityMap.find(uuid);
		if (it != m_EntityMap.end())
			return { it->second, this };

		return {};
	}

	void Scene::OnRuntimeStart()
	{
		ARC_PROFILE_SCOPE()

		SortForSprites();

		m_IsRunning = true;

		m_PhysicsFrameAccumulator = 0.0f;

		#pragma region Physics
		{
			ARC_PROFILE_CATEGORY("Physics", Profile::Category::Physics)

			#pragma region Physics3D
			{
				Physics3D::Init();
				m_BodyActivationListener3D = new Physics3DBodyActivationListener();
				m_ContactListener3D = new Physics3DContactListener();
				JPH::PhysicsSystem& physicsSystem = Physics3D::GetPhysicsSystem();
				physicsSystem.SetBodyActivationListener(m_BodyActivationListener3D);
				physicsSystem.SetContactListener(m_ContactListener3D);

				auto group = m_Registry.group<RigidbodyComponent>(entt::get<TransformComponent>);
				for (auto &&[e, rb, tc] : group.each())
				{
					rb.PreviousTranslation = rb.Translation = tc.Translation;
					rb.PreviousRotation = rb.Rotation = tc.Rotation;
					CreateRigidbody({ e, this }, tc, rb);
				}

				physicsSystem.OptimizeBroadPhase();
			}
			#pragma endregion

			#pragma region Physics2D
			{
				m_PhysicsWorld2D = new b2World({ Gravity.x, Gravity.y });
				m_ContactListener2D = new Physics2DContactListener(this);
				m_PhysicsWorld2D->SetContactListener(m_ContactListener2D);

				{
					auto group = m_Registry.group<Rigidbody2DComponent>(entt::get<TransformComponent>);
					for (auto &&[e, rb, tc] : group.each())
					{
						CreateRigidbody2D({ e, this }, tc, rb);
						rb.PreviousTranslationRotation = rb.TranslationRotation = { tc.Translation.x, tc.Translation.y, tc.Rotation.z };
					}
				}

				{
					auto distanceJointGroup = m_Registry.group<DistanceJoint2DComponent>(entt::get<Rigidbody2DComponent>);
					for (auto &&[e, joint, body] : distanceJointGroup.each())
					{
						Entity connectedBodyEntity = GetEntity(joint.ConnectedRigidbody);
						if (connectedBodyEntity && connectedBodyEntity.HasComponent<Rigidbody2DComponent>())
						{
							auto body1 = static_cast<b2Body*>(body.RuntimeBody);
							auto body2 = static_cast<b2Body*>(connectedBodyEntity.GetComponent<Rigidbody2DComponent>().RuntimeBody);

							b2Vec2 worldAnchorA = body1->GetWorldPoint({ joint.Anchor.x, joint.Anchor.y });
							b2Vec2 worldAnchorB = body2->GetWorldPoint({ joint.ConnectedAnchor.x, joint.ConnectedAnchor.y });

							b2DistanceJointDef jd;
							jd.Initialize(body1, body2, worldAnchorA, worldAnchorB);
							jd.collideConnected = joint.EnableCollision;
							if (!joint.AutoDistance)
								jd.length = joint.Distance;
							jd.minLength = glm::min(jd.length, joint.MinDistance);
							jd.maxLength = jd.length + glm::max(joint.MaxDistanceBy, 0.0f);

							joint.RuntimeJoint = m_PhysicsWorld2D->CreateJoint(&jd);
						}
					}

					auto springJointView = m_Registry.group<SpringJoint2DComponent>(entt::get<Rigidbody2DComponent>);
					for (auto &&[e, joint, body] : springJointView.each())
					{
						Entity connectedBodyEntity = GetEntity(joint.ConnectedRigidbody);
						if (connectedBodyEntity && connectedBodyEntity.HasComponent<Rigidbody2DComponent>())
						{
							auto body1 = static_cast<b2Body*>(body.RuntimeBody);
							auto body2 = static_cast<b2Body*>(connectedBodyEntity.GetComponent<Rigidbody2DComponent>().RuntimeBody);

							b2Vec2 worldAnchorA = body1->GetWorldPoint({ joint.Anchor.x, joint.Anchor.y });
							b2Vec2 worldAnchorB = body2->GetWorldPoint({ joint.ConnectedAnchor.x, joint.ConnectedAnchor.y });

							b2DistanceJointDef jd;
							jd.Initialize(body1, body2, worldAnchorA, worldAnchorB);
							jd.collideConnected = joint.EnableCollision;
							if (!joint.AutoDistance)
								jd.length = joint.Distance;
							jd.minLength = glm::min(jd.length, joint.MinDistance);
							jd.maxLength = jd.length + glm::max(joint.MaxDistanceBy, 0.0f);
							b2LinearStiffness(jd.stiffness, jd.damping, joint.Frequency, joint.DampingRatio, body1, body2);

							joint.RuntimeJoint = m_PhysicsWorld2D->CreateJoint(&jd);
						}
					}

					auto hingeJointView = m_Registry.group<HingeJoint2DComponent>(entt::get<Rigidbody2DComponent>);
					for (auto &&[e, joint, body] : hingeJointView.each())
					{
						Entity connectedBodyEntity = GetEntity(joint.ConnectedRigidbody);
						if (connectedBodyEntity && connectedBodyEntity.HasComponent<Rigidbody2DComponent>())
						{
							auto body1 = static_cast<b2Body*>(body.RuntimeBody);
							auto body2 = static_cast<b2Body*>(connectedBodyEntity.GetComponent<Rigidbody2DComponent>().RuntimeBody);

							b2RevoluteJointDef jd;
							jd.Initialize(body1, body2, body1->GetWorldPoint({ joint.Anchor.x, joint.Anchor.y }));
							jd.collideConnected = joint.EnableCollision;
							jd.enableLimit = joint.UseLimits;
							jd.lowerAngle = joint.LowerAngle;
							jd.upperAngle = joint.UpperAngle;
							jd.enableMotor = joint.UseMotor;
							jd.motorSpeed = joint.MotorSpeed;
							jd.maxMotorTorque = joint.MaxMotorTorque;

							joint.RuntimeJoint = m_PhysicsWorld2D->CreateJoint(&jd);
						}
					}

					auto sliderJointView = m_Registry.group<SliderJoint2DComponent>(entt::get<Rigidbody2DComponent>);
					for (auto &&[e, joint, body] : sliderJointView.each())
					{
						Entity connectedBodyEntity = GetEntity(joint.ConnectedRigidbody);
						if (connectedBodyEntity && connectedBodyEntity.HasComponent<Rigidbody2DComponent>())
						{
							auto body1 = static_cast<b2Body*>(body.RuntimeBody);
							auto body2 = static_cast<b2Body*>(connectedBodyEntity.GetComponent<Rigidbody2DComponent>().RuntimeBody);

							b2Vec2 worldAxis(1.0f, 0.0f);

							b2PrismaticJointDef jd;
							jd.Initialize(body1, body2, body1->GetWorldPoint({ joint.Anchor.x, joint.Anchor.y }), worldAxis);
							jd.collideConnected = joint.EnableCollision;
							jd.referenceAngle = joint.Angle;
							jd.enableLimit = joint.UseLimits;
							jd.lowerTranslation = joint.LowerTranslation;
							jd.upperTranslation = joint.UpperTranslation;
							jd.enableMotor = joint.UseMotor;
							jd.motorSpeed = joint.MotorSpeed;
							jd.maxMotorForce = joint.MaxMotorForce;

							joint.RuntimeJoint = m_PhysicsWorld2D->CreateJoint(&jd);
						}
					}

					auto wheelJointView = m_Registry.group<WheelJoint2DComponent>(entt::get<Rigidbody2DComponent>);
					for (auto &&[e, joint, body] : wheelJointView.each())
					{
						Entity connectedBodyEntity = GetEntity(joint.ConnectedRigidbody);
						if (connectedBodyEntity && connectedBodyEntity.HasComponent<Rigidbody2DComponent>())
						{
							auto body1 = static_cast<b2Body*>(body.RuntimeBody);
							auto body2 = static_cast<b2Body*>(connectedBodyEntity.GetComponent<Rigidbody2DComponent>().RuntimeBody);

							b2Vec2 axis(0.0f, 1.0f);

							float mass = body1->GetMass();
							float omega = 2.0f * b2_pi * joint.Frequency;

							b2WheelJointDef jd;
							jd.Initialize(body1, body2, body1->GetWorldPoint({ joint.Anchor.x, joint.Anchor.y }), axis);
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
		}
		#pragma endregion

		#pragma region Audio
		{
			ARC_PROFILE_CATEGORY("Audio", Profile::Category::Audio)

			auto listenerView = m_Registry.group<AudioListenerComponent>(entt::get<TransformComponent>);
			for (auto &&[e, ac, tc] : listenerView.each())
			{
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

			auto sourceView = m_Registry.group<AudioSourceComponent>(entt::get<TransformComponent>);
			for (auto &&[e, ac, tc] : sourceView.each())
			{
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

		#pragma region VFX
		{
			ARC_PROFILE_SCOPE("Submit Particle Data")

			auto particleSystemView = m_Registry.view<ParticleSystemComponent>();
			for (auto&& [e, psc] : particleSystemView.each())
			{
				if (psc.System->GetProperties().PlayOnAwake)
					psc.System->Play();
				else
					psc.System->Stop(true);
			}
		}
		#pragma endregion

		#pragma region Scripting
		{
			ARC_PROFILE_CATEGORY("OnCreate", Profile::Category::Script)

			auto scriptView = m_Registry.view<ScriptComponent>();
			for (auto &&[e, sc] : scriptView.each())
			{
				Entity entity = { e, this };
				ARC_PROFILE_TAG("Entity", entity.GetTag().data())
				ARC_PROFILE_TAG("EntityID", entity.GetUUID())

				for (const auto& className : sc.Classes)
				{
					ARC_PROFILE_TAG("ScriptInstantiate", className.c_str())

					ScriptEngine::CreateInstance(entity, className);
				}

				for (const auto& className : sc.Classes)
				{
					ARC_PROFILE_TAG("Script", className.c_str())

					ScriptEngine::GetInstance(entity, className)->InvokeOnCreate();
				}
			}
		}
		#pragma endregion
	}

	void Scene::OnRuntimeStop()
	{
		ARC_PROFILE_SCOPE()

		m_IsRunning = false;

		#pragma region Scripting
		{
			ARC_PROFILE_CATEGORY("OnDestroy", Profile::Category::Script)

			auto scriptView = m_Registry.view<ScriptComponent>();
			for (auto &&[e, sc] : scriptView.each())
			{
				Entity entity = { e, this };
				ARC_PROFILE_TAG("Entity", entity.GetTag().data())
				ARC_PROFILE_TAG("EntityID", entity.GetUUID())

				for (const auto& className : sc.Classes)
				{
					ARC_PROFILE_TAG("Script", className.c_str())

					ScriptEngine::GetInstance(entity, className)->InvokeOnDestroy();
					ScriptEngine::RemoveInstance(entity, className);
				}

				sc.Classes.clear();
			}
		}
		#pragma endregion

		#pragma region Audio
		{
			ARC_PROFILE_CATEGORY("Audio", Profile::Category::Audio)

			auto view = m_Registry.view<AudioSourceComponent>();
			for (auto &&[e, ac] : view.each())
			{
				if (ac.Source)
					ac.Source->Stop();
			}
		}
		#pragma endregion

		#pragma region Physics
		{
			ARC_PROFILE_CATEGORY("Physics", Profile::Category::Physics)

			#pragma region Physics3D
			{
				JPH::BodyInterface& bodyInterface = Physics3D::GetPhysicsSystem().GetBodyInterface();
				auto view = m_Registry.view<RigidbodyComponent>();
				for (auto &&[e, rb] : view.each())
				{
					if (rb.RuntimeBody)
					{
						const auto* body = static_cast<const JPH::Body*>(rb.RuntimeBody);
						bodyInterface.RemoveBody(body->GetID());
						bodyInterface.DestroyBody(body->GetID());
					}
				}

				delete m_BodyActivationListener3D;
				delete m_ContactListener3D;
				m_BodyActivationListener3D = nullptr;
				m_ContactListener3D = nullptr;
				Physics3D::Shutdown();
			}
			#pragma endregion

			#pragma region Physics2D
			{
				delete m_ContactListener2D;
				delete m_PhysicsWorld2D;
				m_ContactListener2D = nullptr;
				m_PhysicsWorld2D = nullptr;
			}
			#pragma endregion
		}
		#pragma endregion
	}

	void Scene::OnUpdateEditor([[maybe_unused]] Timestep ts, const Ref<RenderGraphData>& renderGraphData, const EditorCamera& camera)
	{
		ARC_PROFILE_SCOPE()

		#pragma region VFX
		{
			ARC_PROFILE_CATEGORY("VFX", Profile::Category::VFX)

			auto particleSystemView = m_Registry.view<TransformComponent, ParticleSystemComponent>();
			for (auto&& [e, tc, psc] : particleSystemView.each())
				psc.System->OnUpdate(ts, tc.Translation);
		}
		#pragma endregion

		CameraData cameraData =
		{
			camera.GetView(),
			camera.GetProjection(),
			camera.GetViewProjection(),
			camera.GetPosition()
		};
		OnRender(renderGraphData, cameraData);
	}

	void Scene::OnUpdateRuntime([[maybe_unused]] Timestep ts, const Ref<RenderGraphData>& renderGraphData, const EditorCamera* overrideCamera)
	{
		ARC_PROFILE_SCOPE()

		#pragma region Scripting
		{
			ARC_PROFILE_CATEGORY("OnUpdate", Profile::Category::Script)

			auto scriptView = m_Registry.view<ScriptComponent>();
			for (auto &&[e, sc] : scriptView.each())
			{
				Entity entity = { e, this };
				ARC_PROFILE_TAG("Entity", entity.GetTag().data())
				ARC_PROFILE_TAG("EntityID", entity.GetUUID())

				for (const auto& className : sc.Classes)
				{
					ARC_PROFILE_TAG("Script", className.c_str())

					ScriptEngine::GetInstance(entity, className)->InvokeOnUpdate(ts);
				}
			}
		}
		#pragma endregion
		
		#pragma region Physics
		{
			ARC_PROFILE_CATEGORY("Physics", Profile::Category::Physics)

			// Minimum stable value is 16.0
			constexpr float physicsStepRate = 50.0f;
			constexpr float physicsTs = 1.0f / physicsStepRate;

			bool stepped = false;
			m_PhysicsFrameAccumulator += ts;

			while (m_PhysicsFrameAccumulator >= physicsTs)
			{
				m_ContactListener2D->OnUpdate(physicsTs);
				m_PhysicsWorld2D->Step(physicsTs, static_cast<int32_t>(VelocityIterations), static_cast<int32_t>(PositionIterations));

				Physics3D::Step(physicsTs);

				m_PhysicsFrameAccumulator -= physicsTs;
				stepped = true;
			}

			float interpolationFactor = m_PhysicsFrameAccumulator / physicsTs;

			#pragma region Physics3D
			{
				const auto& bodyInterface = Physics3D::GetPhysicsSystem().GetBodyInterface();
				auto view = m_Registry.group<RigidbodyComponent>(entt::get<TransformComponent>);
				for (auto &&[e, rb, tc] : view.each())
				{
					if (!rb.RuntimeBody)
						continue;

					auto body = static_cast<const JPH::Body*>(rb.RuntimeBody);

					if (!bodyInterface.IsActive(body->GetID()))
						continue;

					if (rb.Interpolation)
					{
						if (stepped)
						{
							JPH::Vec3 position = body->GetPosition();
							JPH::Vec3 rotation = body->GetRotation().GetEulerAngles();

							rb.PreviousTranslation = rb.Translation;
							rb.PreviousRotation = rb.Rotation;
							rb.Translation = { position.GetX(), position.GetY(), position.GetZ() };
							rb.Rotation = glm::vec3(rotation.GetX(), rotation.GetY(), rotation.GetZ());
						}

						tc.Translation = glm::lerp(rb.PreviousTranslation, rb.Translation, interpolationFactor);
						tc.Rotation = glm::eulerAngles(glm::slerp(rb.PreviousRotation, rb.Rotation, interpolationFactor));
					}
					else
					{
						JPH::Vec3 position = body->GetPosition();
						JPH::Vec3 rotation = body->GetRotation().GetEulerAngles();

						rb.PreviousTranslation = rb.Translation;
						rb.PreviousRotation = rb.Rotation;
						rb.Translation = { position.GetX(), position.GetY(), position.GetZ() };
						rb.Rotation = glm::vec3(rotation.GetX(), rotation.GetY(), rotation.GetZ());
						tc.Translation = rb.Translation;
						tc.Rotation = glm::eulerAngles(rb.Rotation);
					}
				}
			}
			#pragma endregion

			#pragma region Physics2D
			{
				auto view = m_Registry.group<Rigidbody2DComponent>(entt::get<TransformComponent>);
				for (auto &&[e, rb, tc] : view.each())
				{
					const auto* body = static_cast<b2Body*>(rb.RuntimeBody);

					if (!body->IsAwake())
						continue;

					if (rb.Interpolation)
					{
						if (stepped)
						{
							b2Vec2 position = body->GetPosition();
							rb.PreviousTranslationRotation = rb.TranslationRotation;
							rb.TranslationRotation = { position.x, position.y, body->GetAngle() };
						}

						glm::vec3 lerpedTranslationRotation = glm::lerp(rb.PreviousTranslationRotation, rb.TranslationRotation, interpolationFactor);
						tc.Translation.x = lerpedTranslationRotation.x;
						tc.Translation.y = lerpedTranslationRotation.y;
						tc.Rotation.z = lerpedTranslationRotation.z;
					}
					else
					{
						b2Vec2 position = body->GetPosition();

						rb.PreviousTranslationRotation = rb.TranslationRotation;
						rb.TranslationRotation = { position.x, position.y, body->GetAngle() };

						tc.Translation.x = rb.TranslationRotation.x;
						tc.Translation.y = rb.TranslationRotation.y;
						tc.Rotation.z = rb.TranslationRotation.z;
					}
				}

				auto distanceJointView = m_Registry.view<DistanceJoint2DComponent>();
				for (auto &&[e, joint] : distanceJointView.each())
				{
					if (joint.RuntimeJoint)
					{
						auto j = static_cast<b2Joint*>(joint.RuntimeJoint);

						if (j->GetReactionForce(physicsStepRate).LengthSquared() > joint.BreakForce * joint.BreakForce)
						{
							m_PhysicsWorld2D->DestroyJoint(j);
							joint.RuntimeJoint = nullptr;
						}
					}
				}

				auto springJointView = m_Registry.view<SpringJoint2DComponent>();
				for (auto &&[e, joint] : springJointView.each())
				{
					if (joint.RuntimeJoint)
					{
						auto j = static_cast<b2Joint*>(joint.RuntimeJoint);

						if (j->GetReactionForce(physicsStepRate).LengthSquared() > joint.BreakForce * joint.BreakForce)
						{
							m_PhysicsWorld2D->DestroyJoint(j);
							joint.RuntimeJoint = nullptr;
						}
					}
				}

				auto hingeJointView = m_Registry.view<HingeJoint2DComponent>();
				for (auto &&[e, joint] : hingeJointView.each())
				{
					if (joint.RuntimeJoint)
					{
						auto j = static_cast<b2Joint*>(joint.RuntimeJoint);

						if (j->GetReactionForce(physicsStepRate).LengthSquared() > joint.BreakForce * joint.BreakForce
							|| j->GetReactionTorque(physicsStepRate) > joint.BreakTorque)
						{
							m_PhysicsWorld2D->DestroyJoint(j);
							joint.RuntimeJoint = nullptr;
						}
					}
				}

				auto sliderJointView = m_Registry.view<SliderJoint2DComponent>();
				for (auto &&[e, joint] : sliderJointView.each())
				{
					if (joint.RuntimeJoint)
					{
						auto j = static_cast<b2Joint*>(joint.RuntimeJoint);

						if (j->GetReactionForce(physicsStepRate).LengthSquared() > joint.BreakForce * joint.BreakForce
							|| j->GetReactionTorque(physicsStepRate) > joint.BreakTorque)
						{
							m_PhysicsWorld2D->DestroyJoint(j);
							joint.RuntimeJoint = nullptr;
						}
					}
				}

				auto wheelJointView = m_Registry.view<WheelJoint2DComponent>();
				for (auto &&[e, joint] : wheelJointView.each())
				{
					if (joint.RuntimeJoint)
					{
						auto j = static_cast<b2Joint*>(joint.RuntimeJoint);

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
		}
		#pragma endregion

		#pragma region Audio
		{
			ARC_PROFILE_CATEGORY("Audio", Profile::Category::Audio)

			auto listenerView = m_Registry.group<AudioListenerComponent>(entt::get<TransformComponent>);
			for (auto &&[e, ac, tc] : listenerView.each())
			{
				if (ac.Active)
				{
					const glm::mat4 inverted = glm::inverse(Entity(e, this).GetWorldTransform());
					const glm::vec3 forward = normalize(glm::vec3(inverted[2]));
					ac.Listener->SetPosition(tc.Translation);
					ac.Listener->SetDirection(-forward);
					break;
				}
			}

			auto sourceView = m_Registry.group<AudioSourceComponent>(entt::get<TransformComponent>);
			for (auto &&[e, ac, tc] : sourceView.each())
			{
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

		#pragma region VFX
		{
			ARC_PROFILE_CATEGORY("VFX", Profile::Category::VFX)

			auto particleSystemView = m_Registry.view<TransformComponent, ParticleSystemComponent>();
			for (auto&& [e, tc, psc] : particleSystemView.each())
				psc.System->OnUpdate(ts, tc.Translation);
		}
		#pragma endregion

		#pragma region Rendering
		CameraData cameraData = {};
		{
			ARC_PROFILE_CATEGORY("Camera", Profile::Category::Camera)

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
				cameraData.View = overrideCamera->GetView();
				cameraData.Projection = overrideCamera->GetProjection();
				cameraData.ViewProjection = overrideCamera->GetViewProjection();
				cameraData.Position = overrideCamera->GetPosition();
			}
		}

		

		OnRender(renderGraphData, cameraData);
		#pragma endregion
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		ARC_PROFILE_SCOPE()

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
		ARC_PROFILE_CATEGORY("Camera", Profile::Category::Camera)

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return { entity, this };
		}
		return {};
	}

	void Scene::SortForSprites()
	{
		m_Registry.sort<SpriteRendererComponent>([](const auto& lhs, const auto& rhs)
		{
			return lhs.SortingOrder < rhs.SortingOrder;
		});
	}

	void Scene::OnRender(const Ref<RenderGraphData>& renderGraphData, const CameraData& cameraData)
	{
		ARC_PROFILE_CATEGORY("Rendering", Profile::Category::Rendering)

		std::vector<Entity> lights;
		{
			ARC_PROFILE_SCOPE("Prepare Light Data")

			auto view = m_Registry.view<LightComponent>();
			lights.reserve(view.size());
			for (auto &&[entity, lc] : view.each())
				lights.emplace_back(Entity(entity, this));
		}
		Entity skylight = {};
		{
			ARC_PROFILE_SCOPE("PrepareSkylightData")

			auto view = m_Registry.view<SkyLightComponent>();
			if (!view.empty())
				skylight = Entity(*view.begin(), this);
		}

		Renderer3D::BeginScene(cameraData, skylight, std::move(lights));
		// Meshes
		{
			ARC_PROFILE_SCOPE("Submit Mesh Data")

			auto view = m_Registry.view<MeshComponent>();
			Renderer3D::ReserveMeshes(view.size());
			for (auto &&[entity, meshComponent] : view.each())
			{
				if (meshComponent.MeshGeometry && meshComponent.MeshGeometry->GetSubmeshCount() != 0)
				{
					ARC_CORE_ASSERT(meshComponent.MeshGeometry->GetSubmeshCount() > meshComponent.SubmeshIndex, "Trying to access submesh index that does not exist!")
					Renderer3D::SubmitMesh(Entity(entity, this).GetWorldTransform(), meshComponent.MeshGeometry->GetSubmesh(meshComponent.SubmeshIndex), meshComponent.CullMode);
				}
			}
		}
		Renderer3D::EndScene(renderGraphData);

		Renderer2D::BeginScene(cameraData.ViewProjection);
		{
			ARC_PROFILE_SCOPE("Submit Particle Data")

			auto particleSystemView = m_Registry.view<ParticleSystemComponent>();
			for (auto&& [e, psc] : particleSystemView.each())
				psc.System->OnRender();
		}
		{
			ARC_PROFILE_SCOPE("Submit 2D Data")

			auto view = m_Registry.view<SpriteRendererComponent>();
			for (auto &&[entity, sprite] : view.each())
			{
				Renderer2D::DrawQuad(Entity(entity, this).GetWorldTransform(), sprite.Texture, sprite.Color, sprite.TilingFactor);
			}
		}
		Renderer2D::EndScene(renderGraphData);
	}

	void Scene::CreateRigidbody(Entity entity, const TransformComponent& transform, RigidbodyComponent& component) const
	{
		ARC_PROFILE_SCOPE()
		ARC_PROFILE_TAG("Entity", entity.GetTag().data())
		ARC_PROFILE_TAG("EntityID", entity.GetUUID())

		if (!m_IsRunning)
			return;

		auto& bodyInterface = Physics3D::GetPhysicsSystem().GetBodyInterface();
		if (component.RuntimeBody)
		{
			bodyInterface.DestroyBody(static_cast<JPH::Body*>(component.RuntimeBody)->GetID());
			component.RuntimeBody = nullptr;
		}

		JPH::MutableCompoundShapeSettings compoundShapeSettings;
		float maxScaleComponent = glm::max(glm::max(transform.Scale.x, transform.Scale.y), transform.Scale.z);

		const auto& entityName = entity.GetComponent<TagComponent>().Tag;

		if (entity.HasComponent<BoxColliderComponent>())
		{
			const auto& bc = entity.GetComponent<BoxColliderComponent>();
			const auto* mat = new PhysicsMaterial3D(entityName, JPH::ColorArg(255, 0, 0), bc.Friction, bc.Restitution);
			
			glm::vec3 scale = bc.Size * transform.Scale * 2.0f;
			JPH::BoxShapeSettings shapeSettings({ glm::abs(scale.x), glm::abs(scale.y), glm::abs(scale.z) }, 0.05f, mat);
			shapeSettings.SetDensity(glm::max(0.001f, bc.Density));

			compoundShapeSettings.AddShape({ bc.Offset.x, bc.Offset.y, bc.Offset.z }, JPH::Quat::sIdentity(), shapeSettings.Create().Get());
		}

		if (entity.HasComponent<SphereColliderComponent>())
		{
			const auto& sc = entity.GetComponent<SphereColliderComponent>();
			const auto* mat = new PhysicsMaterial3D(entityName, JPH::ColorArg(255, 0, 0), sc.Friction, sc.Restitution);

			float radius = 2.0f * sc.Radius * maxScaleComponent;
			JPH::SphereShapeSettings shapeSettings(glm::max(0.01f, radius), mat);
			shapeSettings.SetDensity(glm::max(0.001f, sc.Density));

			compoundShapeSettings.AddShape({ sc.Offset.x, sc.Offset.y, sc.Offset.z }, JPH::Quat::sIdentity(), shapeSettings.Create().Get());
		}

		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			const auto& cc = entity.GetComponent<CapsuleColliderComponent>();
			const auto* mat = new PhysicsMaterial3D(entityName, JPH::ColorArg(255, 0, 0), cc.Friction, cc.Restitution);

			float radius = 2.0f * cc.Radius * maxScaleComponent;
			JPH::CapsuleShapeSettings shapeSettings(glm::max(0.01f, cc.Height) * 0.5f, glm::max(0.01f, radius), mat);
			shapeSettings.SetDensity(glm::max(0.001f, cc.Density));

			compoundShapeSettings.AddShape({ cc.Offset.x, cc.Offset.y, cc.Offset.z }, JPH::Quat::sIdentity(), shapeSettings.Create().Get());
		}

		if (entity.HasComponent<TaperedCapsuleColliderComponent>())
		{
			const auto& tcc = entity.GetComponent<TaperedCapsuleColliderComponent>();
			const auto* mat = new PhysicsMaterial3D(entityName, JPH::ColorArg(255, 0, 0), tcc.Friction, tcc.Restitution);

			float topRadius = 2.0f * tcc.TopRadius * maxScaleComponent;
			float bottomRadius = 2.0f * tcc.BottomRadius * maxScaleComponent;
			JPH::TaperedCapsuleShapeSettings shapeSettings(glm::max(0.01f, tcc.Height) * 0.5f, glm::max(0.01f, topRadius), glm::max(0.01f, bottomRadius), mat);
			shapeSettings.SetDensity(glm::max(0.001f, tcc.Density));

			compoundShapeSettings.AddShape({ tcc.Offset.x, tcc.Offset.y, tcc.Offset.z }, JPH::Quat::sIdentity(), shapeSettings.Create().Get());
		}

		if (entity.HasComponent<CylinderColliderComponent>())
		{
			const auto& cc = entity.GetComponent<CylinderColliderComponent>();
			const auto* mat = new PhysicsMaterial3D(entityName, JPH::ColorArg(255, 0, 0), cc.Friction, cc.Restitution);

			float radius = 2.0f * cc.Radius * maxScaleComponent;
			JPH::CylinderShapeSettings shapeSettings(glm::max(0.01f, cc.Height) * 0.5f, glm::max(0.01f, radius), 0.05f, mat);
			shapeSettings.SetDensity(glm::max(0.001f, cc.Density));

			compoundShapeSettings.AddShape({ cc.Offset.x, cc.Offset.y, cc.Offset.z }, JPH::Quat::sIdentity(), shapeSettings.Create().Get());
		}

		// Body
		auto rotation = glm::quat(transform.Rotation);
		
		auto layer = entity.GetComponent<TagComponent>().Layer;
		uint8_t layerIndex = 1;	// Default Layer
		auto collisionMaskIt = LayerCollisionMask.find(layer);
		if (collisionMaskIt != LayerCollisionMask.end())
			layerIndex = collisionMaskIt->second.Index;

		JPH::BodyCreationSettings bodySettings(compoundShapeSettings.Create().Get(), {transform.Translation.x, transform.Translation.y, transform.Translation.z}, {rotation.x, rotation.y, rotation.z, rotation.w}, static_cast<JPH::EMotionType>(component.Type), layerIndex);

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

		JPH::Body* body = bodyInterface.CreateBody(bodySettings);

		JPH::EActivation activation = component.Awake && component.Type != RigidbodyComponent::BodyType::Static ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;
		bodyInterface.AddBody(body->GetID(), activation);

		component.RuntimeBody = body;
	}

	void Scene::CreateRigidbody2D(Entity entity, const TransformComponent& transform, Rigidbody2DComponent& component) const
	{
		ARC_PROFILE_SCOPE()
		ARC_PROFILE_TAG("Entity", entity.GetTag().data())
		ARC_PROFILE_TAG("EntityID", entity.GetUUID())

		if (!m_PhysicsWorld2D)
			return;

		b2BodyDef def;
		def.type = static_cast<b2BodyType>(component.Type);
		def.linearDamping = glm::max(component.LinearDrag, 0.0f);
		def.angularDamping = glm::max(component.AngularDrag, 0.0f);
		def.allowSleep = component.AllowSleep;
		def.awake = component.Awake;
		def.fixedRotation = component.FreezeRotation;
		def.bullet = component.Continuous;
		def.gravityScale = component.GravityScale;

		def.position.Set(transform.Translation.x, transform.Translation.y);
		def.angle = transform.Rotation.z;

		b2Body* rb = m_PhysicsWorld2D->CreateBody(&def);
		component.RuntimeBody = rb;

		if (entity.HasComponent<BoxCollider2DComponent>())
			CreateBoxCollider2D(entity, transform, component, entity.GetComponent<BoxCollider2DComponent>());

		if (entity.HasComponent<CircleCollider2DComponent>())
			CreateCircleCollider2D(entity, transform, component, entity.GetComponent<CircleCollider2DComponent>());

		if (entity.HasComponent<PolygonCollider2DComponent>())
			CreatePolygonCollider2D(entity, component, entity.GetComponent<PolygonCollider2DComponent>());

		if (!component.AutoMass && component.Mass > 0.01f)
		{
			b2MassData massData = rb->GetMassData();
			massData.mass = component.Mass;
			rb->SetMassData(&massData);
		}
	}

	void Scene::CreateBoxCollider2D(Entity entity, const TransformComponent& transform, const Rigidbody2DComponent& rb, BoxCollider2DComponent& component) const
	{
		ARC_PROFILE_SCOPE()
		ARC_PROFILE_TAG("Entity", entity.GetTag().data())
		ARC_PROFILE_TAG("EntityID", entity.GetUUID())

		if (!m_PhysicsWorld2D)
			return;

		b2PolygonShape boxShape;
		boxShape.SetAsBox(component.Size.x * transform.Scale.x, component.Size.y * transform.Scale.y, { component.Offset.x, component.Offset.y }, 0.0f);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &boxShape;
		fixtureDef.isSensor = component.IsSensor;
		fixtureDef.density = component.Density;
		fixtureDef.friction = component.Friction;
		fixtureDef.restitution = component.Restitution;
		fixtureDef.userData.pointer = static_cast<uint32_t>(entity);

		auto layer = entity.GetComponent<TagComponent>().Layer;
		auto collisionMaskIt = LayerCollisionMask.find(layer);
		if (collisionMaskIt == LayerCollisionMask.end())
			layer = Scene::DefaultLayer;
		fixtureDef.filter.categoryBits = layer;
		fixtureDef.filter.maskBits = LayerCollisionMask[layer].Flags;

		auto* body = static_cast<b2Body*>(rb.RuntimeBody);
		b2Fixture* fixture = body->CreateFixture(&fixtureDef);
		component.RuntimeFixture = fixture;
	}

	void Scene::CreateCircleCollider2D(Entity entity, const TransformComponent& transform, const Rigidbody2DComponent& rb, CircleCollider2DComponent& component) const
	{
		ARC_PROFILE_SCOPE()
		ARC_PROFILE_TAG("Entity", entity.GetTag().data())
		ARC_PROFILE_TAG("EntityID", entity.GetUUID())

		if (!m_PhysicsWorld2D)
			return;

		b2CircleShape circleShape;
		circleShape.m_radius = component.Radius * glm::max(transform.Scale.x, transform.Scale.y);
		circleShape.m_p = { component.Offset.x, component.Offset.y };

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &circleShape;
		fixtureDef.isSensor = component.IsSensor;
		fixtureDef.density = component.Density;
		fixtureDef.friction = component.Friction;
		fixtureDef.restitution = component.Restitution;
		fixtureDef.userData.pointer = static_cast<uint32_t>(entity);

		auto layer = entity.GetComponent<TagComponent>().Layer;
		auto collisionMaskIt = LayerCollisionMask.find(layer);
		if (collisionMaskIt == LayerCollisionMask.end())
			layer = Scene::DefaultLayer;
		fixtureDef.filter.categoryBits = layer;
		fixtureDef.filter.maskBits = LayerCollisionMask[layer].Flags;

		auto* body = static_cast<b2Body*>(rb.RuntimeBody);
		b2Fixture* fixture = body->CreateFixture(&fixtureDef);
		component.RuntimeFixture = fixture;
	}

	void Scene::CreatePolygonCollider2D(Entity entity, const Rigidbody2DComponent& rb, PolygonCollider2DComponent& component) const
	{
		ARC_PROFILE_SCOPE()
		ARC_PROFILE_TAG("Entity", entity.GetTag().data())
		ARC_PROFILE_TAG("EntityID", entity.GetUUID())

		if (!m_PhysicsWorld2D)
			return;

		if (component.Points.size() < 3)
		{
			ARC_CORE_ERROR("Cannot create PolygonCollider2D with {} points", component.Points.size());
			return;
		}

		b2PolygonShape shape;
		shape.Set(reinterpret_cast<const b2Vec2*>(component.Points.data()), static_cast<int32_t>(component.Points.size()));

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;
		fixtureDef.isSensor = component.IsSensor;
		fixtureDef.density = component.Density;
		fixtureDef.friction = component.Friction;
		fixtureDef.restitution = component.Restitution;
		fixtureDef.userData.pointer = static_cast<uint32_t>(entity);

		auto layer = entity.GetComponent<TagComponent>().Layer;
		auto collisionMaskIt = LayerCollisionMask.find(layer);
		if (collisionMaskIt == LayerCollisionMask.end())
			layer = Scene::DefaultLayer;
		fixtureDef.filter.categoryBits = layer;
		fixtureDef.filter.maskBits = LayerCollisionMask[layer].Flags;

		auto* body = static_cast<b2Body*>(rb.RuntimeBody);
		b2Fixture* fixture = body->CreateFixture(&fixtureDef);
		component.RuntimeFixture = fixture;
	}

	template<typename T>
	void Scene::OnComponentAdded([[maybe_unused]] Entity entity, [[maybe_unused]] T& component)
	{
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
	void Scene::OnComponentAdded<ParticleSystemComponent>(Entity entity, ParticleSystemComponent& component)
	{
		/* On ParticleSystemComponent added */
	}

	template<>
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& body)
	{
		/* On Rigidbody2DComponent added */
		CreateRigidbody2D(entity, entity.GetComponent<TransformComponent>(), body);
	}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& bc2d)
	{
		/* On BoxCollider2DComponent added */
		if (entity.HasComponent<Rigidbody2DComponent>())
			CreateBoxCollider2D(entity, entity.GetComponent<TransformComponent>(), entity.GetComponent<Rigidbody2DComponent>(), bc2d);
	}
	
	template<>
	void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& cc2d)
	{
		if (entity.HasComponent<Rigidbody2DComponent>())
			CreateCircleCollider2D(entity, entity.GetComponent<TransformComponent>(), entity.GetComponent<Rigidbody2DComponent>(), cc2d);
	}

	template<>
	void Scene::OnComponentAdded<PolygonCollider2DComponent>(Entity entity, PolygonCollider2DComponent& pc2d)
	{
		/* On CircleCollider2DComponent added */
		if (entity.HasComponent<Rigidbody2DComponent>())
			CreatePolygonCollider2D(entity, entity.GetComponent<Rigidbody2DComponent>(), pc2d);
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
		CreateRigidbody(entity, entity.GetComponent<TransformComponent>(), body);
	}

	template<>
	void Scene::OnComponentAdded<BoxColliderComponent>(Entity entity, BoxColliderComponent& bc)
	{
		/* On BoxColliderComponent added */
		if (entity.HasComponent<RigidbodyComponent>())
			CreateRigidbody(entity, entity.GetComponent<TransformComponent>(), entity.GetComponent<RigidbodyComponent>());
	}

	template<>
	void Scene::OnComponentAdded<SphereColliderComponent>(Entity entity, SphereColliderComponent& sc)
	{
		/* On SphereColliderComponent added */
		if (entity.HasComponent<RigidbodyComponent>())
			CreateRigidbody(entity, entity.GetComponent<TransformComponent>(), entity.GetComponent<RigidbodyComponent>());
	}

	template<>
	void Scene::OnComponentAdded<CapsuleColliderComponent>(Entity entity, CapsuleColliderComponent& sc)
	{
		/* On SphereColliderComponent added */
		if (entity.HasComponent<RigidbodyComponent>())
			CreateRigidbody(entity, entity.GetComponent<TransformComponent>(), entity.GetComponent<RigidbodyComponent>());
	}

	template<>
	void Scene::OnComponentAdded<TaperedCapsuleColliderComponent>(Entity entity, TaperedCapsuleColliderComponent& sc)
	{
		/* On SphereColliderComponent added */
		if (entity.HasComponent<RigidbodyComponent>())
			CreateRigidbody(entity, entity.GetComponent<TransformComponent>(), entity.GetComponent<RigidbodyComponent>());
	}

	template<>
	void Scene::OnComponentAdded<CylinderColliderComponent>(Entity entity, CylinderColliderComponent& sc)
	{
		/* On SphereColliderComponent added */
		if (entity.HasComponent<RigidbodyComponent>())
			CreateRigidbody(entity, entity.GetComponent<TransformComponent>(), entity.GetComponent<RigidbodyComponent>());
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
