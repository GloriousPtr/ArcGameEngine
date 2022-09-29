#include "arcpch.h"
#include "Physics3D.h"

#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

#include "PhysicsMaterial3D.h"
#include "Arc/Scene/Scene.h"

namespace ArcEngine
{
	static bool Physics3DObjectCanCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2)
	{
		EntityLayer layer1 = BIT(inObject1);
		EntityLayer layer2 = BIT(inObject2);
		return	(layer1 & Scene::LayerCollisionMask.at(layer2).Flags) == layer1 &&
				(layer2 & Scene::LayerCollisionMask.at(layer1).Flags) == layer2;
	};

	namespace BroadPhaseLayers
	{
		static constexpr JPH::BroadPhaseLayer STATIC(0);
		static constexpr JPH::BroadPhaseLayer DEFAULT(1);
		static constexpr JPH::uint NUM_LAYERS = 3;
	};

	class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
	{
	public:
		BPLayerInterfaceImpl()
		{
			mObjectToBroadPhase[Physics3D::BroadLayer::STATIC]  = BroadPhaseLayers::STATIC;
			mObjectToBroadPhase[Physics3D::BroadLayer::DEFAULT] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3D::BroadLayer::OTHER2]  = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3D::BroadLayer::OTHER3]  = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3D::BroadLayer::OTHER4]  = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3D::BroadLayer::OTHER5]  = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3D::BroadLayer::OTHER6]  = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3D::BroadLayer::OTHER7]  = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3D::BroadLayer::OTHER8]  = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3D::BroadLayer::OTHER9]  = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3D::BroadLayer::OTHER10] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3D::BroadLayer::OTHER11] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3D::BroadLayer::OTHER12] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3D::BroadLayer::OTHER13] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3D::BroadLayer::OTHER14] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3D::BroadLayer::OTHER15] = BroadPhaseLayers::DEFAULT;
		}

		virtual JPH::uint GetNumBroadPhaseLayers() const override
		{
			return BroadPhaseLayers::NUM_LAYERS;
		}

		virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
		{
			ARC_CORE_ASSERT(inLayer < Physics3D::BroadLayer::NUM_LAYERS);
			return mObjectToBroadPhase[inLayer];
		}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
		{
			switch ((BroadPhaseLayer::Type)inLayer)
			{
				case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
				case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
				default:													JPH_ASSERT(false); return "INVALID";
			}
		}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	private:
		JPH::BroadPhaseLayer mObjectToBroadPhase[Physics3D::BroadLayer::NUM_LAYERS];
	};

	static bool Physics3DBroadPhaseCanCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2)
	{
		if (inLayer1 == Physics3D::BroadLayer::STATIC)
			return inLayer2 != BroadPhaseLayers::STATIC;

		return true;
	}

	class Physics3DContactListener : public JPH::ContactListener
	{
	private:
		static void	GetFrictionAndRestitution(const JPH::Body& inBody, const JPH::SubShapeID& inSubShapeID, float& outFriction, float& outRestitution)
		{
			// Get the material that corresponds to the sub shape ID
			const JPH::PhysicsMaterial* material = inBody.GetShape()->GetMaterial(inSubShapeID);
			if (material == JPH::PhysicsMaterial::sDefault)
			{
				outFriction = inBody.GetFriction();
				outRestitution = inBody.GetRestitution();
			}
			else
			{
				const PhysicsMaterial3D* phyMaterial = (const PhysicsMaterial3D*)material;
				outFriction = phyMaterial->Friction;
				outRestitution = phyMaterial->Restitution;
			}
		}

		static void	OverrideContactSettings(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
		{
			// Get the custom friction and restitution for both bodies
			float friction1, friction2, restitution1, restitution2;
			GetFrictionAndRestitution(inBody1, inManifold.mSubShapeID1, friction1, restitution1);
			GetFrictionAndRestitution(inBody2, inManifold.mSubShapeID2, friction2, restitution2);

			// Use the default formulas for combining friction and restitution
			ioSettings.mCombinedFriction = JPH::sqrt(friction1 * friction2);
			ioSettings.mCombinedRestitution = JPH::max(restitution1, restitution2);
		}

	public:
		virtual JPH::ValidateResult OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::CollideShapeResult& inCollisionResult) override
		{
			return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
		}

		virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
		{
			OverrideContactSettings(inBody1, inBody2, inManifold, ioSettings);
		}

		virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
		{
			OverrideContactSettings(inBody1, inBody2, inManifold, ioSettings);
		}

		virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
		{
			/* On Collision Exit */
		}
	};

	class Physics3DBodyActivationListener : public JPH::BodyActivationListener
	{
	public:
		virtual void OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
		{
			/* Body Activated */
		}

		virtual void OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
		{
			/* Body Deactivated */
		}
	};

	JPH::PhysicsSystem* Physics3D::s_PhysicsSystem;
	JPH::TempAllocator* Physics3D::s_TempAllocator;
	JPH::JobSystemThreadPool* Physics3D::s_JobSystem;

	BPLayerInterfaceImpl* Physics3D::s_BPLayerInterface;
	Physics3DContactListener* Physics3D::s_ContactListener;
	Physics3DBodyActivationListener* Physics3D::s_BodyActivationListener;

	void Physics3D::Init()
	{
		JPH::RegisterDefaultAllocator();

		JPH::Factory::sInstance = new JPH::Factory();

		JPH::RegisterTypes();

		s_TempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
		s_JobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, JPH::thread::hardware_concurrency() - 1);
		const JPH::uint cMaxBodies = 65536;
		const JPH::uint cNumBodyMutexes = 0;
		const JPH::uint cMaxBodyPairs = 65536;
		const JPH::uint cMaxContactConstraints = 10240;

		s_BPLayerInterface = new BPLayerInterfaceImpl();
		s_PhysicsSystem = new JPH::PhysicsSystem();
		s_PhysicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, *s_BPLayerInterface, Physics3DBroadPhaseCanCollide, Physics3DObjectCanCollide);

		s_BodyActivationListener = new Physics3DBodyActivationListener();
		s_PhysicsSystem->SetBodyActivationListener(s_BodyActivationListener);

		s_ContactListener = new Physics3DContactListener();
		s_PhysicsSystem->SetContactListener(s_ContactListener);
	}

	void Physics3D::Shutdown()
	{
		delete s_ContactListener;
		s_ContactListener = nullptr;

		delete s_BodyActivationListener;
		s_BodyActivationListener = nullptr;

		delete s_PhysicsSystem;
		s_PhysicsSystem = nullptr;

		delete s_BPLayerInterface;
		s_BPLayerInterface = nullptr;

		delete s_JobSystem;
		s_JobSystem = nullptr;

		delete s_TempAllocator;
		s_TempAllocator = nullptr;

		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;
	}

	void Physics3D::Step(float physicsTs)
	{
		s_PhysicsSystem->Update(physicsTs, 1, 1, s_TempAllocator, s_JobSystem);
	}

	JPH::BodyInterface* Physics3D::GetBodyInterface()
	{
		return &s_PhysicsSystem->GetBodyInterface();
	}

	void Physics3D::OptimizeBroadPhase()
	{
		s_PhysicsSystem->OptimizeBroadPhase();
	}
}
