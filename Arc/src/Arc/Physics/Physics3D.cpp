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

using namespace JPH;

namespace ArcEngine
{
	static bool Physics3DObjectCanCollide(ObjectLayer inObject1, ObjectLayer inObject2)
	{
		switch (inObject1)
		{
			case Physics3D::Layers::NON_MOVING:
				return inObject2 == Physics3D::Layers::MOVING; // Non moving only collides with moving
			case Physics3D::Layers::MOVING:
				return true; // Moving collides with everything
			default:
				JPH_ASSERT(false);
				return false;
		}
	};

	namespace BroadPhaseLayers
	{
		static constexpr BroadPhaseLayer NON_MOVING(0);
		static constexpr BroadPhaseLayer MOVING(1);
		static constexpr uint NUM_LAYERS(2);
	};

	class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
	{
	public:
		BPLayerInterfaceImpl()
		{
			mObjectToBroadPhase[Physics3D::Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
			mObjectToBroadPhase[Physics3D::Layers::MOVING] = BroadPhaseLayers::MOVING;
		}

		virtual uint GetNumBroadPhaseLayers() const override
		{
			return BroadPhaseLayers::NUM_LAYERS;
		}

		virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override
		{
			JPH_ASSERT(inLayer < Physics3D::Layers::NUM_LAYERS);
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
		BroadPhaseLayer mObjectToBroadPhase[Physics3D::Layers::NUM_LAYERS];
	};

	static bool Physics3DBroadPhaseCanCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2)
	{
		switch (inLayer1)
		{
			case Physics3D::Layers::NON_MOVING:
				return inLayer2 == BroadPhaseLayers::MOVING;
			case Physics3D::Layers::MOVING:
				return true;
			default:
				JPH_ASSERT(false);
				return false;
		}
	}

	class Physics3DContactListener : public ContactListener
	{
	public:
		virtual ValidateResult OnContactValidate(const Body& inBody1, const Body& inBody2, const CollideShapeResult& inCollisionResult) override
		{
			return ValidateResult::AcceptAllContactsForThisBodyPair;
		}

		virtual void OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
		{
		}

		virtual void OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
		{
		}

		virtual void OnContactRemoved(const SubShapeIDPair& inSubShapePair) override
		{
		}
	};

	class Physics3DBodyActivationListener : public BodyActivationListener
	{
	public:
		virtual void OnBodyActivated(const BodyID& inBodyID, uint64 inBodyUserData) override
		{
		}

		virtual void OnBodyDeactivated(const BodyID& inBodyID, uint64 inBodyUserData) override
		{
		}
	};

	static BPLayerInterfaceImpl* s_BPLayerInterface;
	static Physics3DContactListener* s_ContactListener;
	static Physics3DBodyActivationListener* s_BodyActivationListener;
	static PhysicsSystem* s_PhysicsSystem;
	static TempAllocator* s_TempAllocator;
	static JobSystemThreadPool* s_JobSystem;
	static uint64_t s_Steps;

	void Physics3D::Init()
	{
		RegisterDefaultAllocator();

		// Install callbacks
//		Trace = TraceImpl;
//		JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

		Factory::sInstance = new Factory();

		RegisterTypes();

		s_TempAllocator = new TempAllocatorImpl(10 * 1024 * 1024);
		s_JobSystem = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);
		const uint cMaxBodies = 65536;
		const uint cNumBodyMutexes = 0;
		const uint cMaxBodyPairs = 65536;
		const uint cMaxContactConstraints = 10240;

		s_BPLayerInterface = new BPLayerInterfaceImpl();
		s_PhysicsSystem = new PhysicsSystem();
		s_PhysicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, *s_BPLayerInterface, Physics3DBroadPhaseCanCollide, Physics3DObjectCanCollide);

		s_BodyActivationListener = new Physics3DBodyActivationListener();
		s_PhysicsSystem->SetBodyActivationListener(s_BodyActivationListener);

		s_ContactListener = new Physics3DContactListener();
		s_PhysicsSystem->SetContactListener(s_ContactListener);

		s_Steps = 0;
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

		delete Factory::sInstance;
		Factory::sInstance = nullptr;
	}

	void Physics3D::Step(float ts)
	{
		++s_Steps;
		const int cCollisionSteps = 1;
		const int cIntegrationSubSteps = 1;
		s_PhysicsSystem->Update(ts, cCollisionSteps, cIntegrationSubSteps, s_TempAllocator, s_JobSystem);
	}

	JPH::BodyInterface* Physics3D::GetBodyInterface()
	{
		return &s_PhysicsSystem->GetBodyInterface();
	}
}
