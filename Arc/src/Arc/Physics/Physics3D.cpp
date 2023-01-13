#include "arcpch.h"
#include "Physics3D.h"

#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "Arc/Scene/Scene.h"

namespace ArcEngine
{
	static bool Physics3DObjectCanCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2)
	{
		ARC_PROFILE_SCOPE()

		const EntityLayer layer1 = BIT(inObject1);
		const EntityLayer layer2 = BIT(inObject2);
		return	(layer1 & Scene::LayerCollisionMask.at(layer2).Flags) == layer1 &&
				(layer2 & Scene::LayerCollisionMask.at(layer1).Flags) == layer2;
	};

	struct Physics3DLayer
	{
		static constexpr uint8_t STATIC = 0;
		static constexpr uint8_t DEFAULT = 1;
		static constexpr uint8_t OTHER2 = 2;
		static constexpr uint8_t OTHER3 = 3;
		static constexpr uint8_t OTHER4 = 4;
		static constexpr uint8_t OTHER5 = 5;
		static constexpr uint8_t OTHER6 = 6;
		static constexpr uint8_t OTHER7 = 7;
		static constexpr uint8_t OTHER8 = 8;
		static constexpr uint8_t OTHER9 = 9;
		static constexpr uint8_t OTHER10 = 10;
		static constexpr uint8_t OTHER11 = 11;
		static constexpr uint8_t OTHER12 = 12;
		static constexpr uint8_t OTHER13 = 13;
		static constexpr uint8_t OTHER14 = 14;
		static constexpr uint8_t OTHER15 = 15;

		static constexpr uint8_t NUM_LAYERS = 16;
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
			ARC_PROFILE_SCOPE()

			mObjectToBroadPhase[Physics3DLayer::STATIC] = BroadPhaseLayers::STATIC;
			mObjectToBroadPhase[Physics3DLayer::DEFAULT] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER2] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER3] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER4] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER5] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER6] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER7] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER8] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER9] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER10] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER11] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER12] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER13] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER14] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER15] = BroadPhaseLayers::DEFAULT;
		}

		[[nodiscard]] JPH::uint GetNumBroadPhaseLayers() const override
		{
			ARC_PROFILE_SCOPE()

			return BroadPhaseLayers::NUM_LAYERS;
		}

		[[nodiscard]] JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
		{
			ARC_PROFILE_SCOPE()

			ARC_CORE_ASSERT(inLayer < Physics3DLayer::NUM_LAYERS)
			return mObjectToBroadPhase[inLayer];
		}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		[[nodiscard]] const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
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
		JPH::BroadPhaseLayer mObjectToBroadPhase[Physics3DLayer::NUM_LAYERS];
	};

	static bool Physics3DBroadPhaseCanCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2)
	{
		ARC_PROFILE_SCOPE()

		if (inLayer1 == Physics3DLayer::STATIC)
			return inLayer2 != BroadPhaseLayers::STATIC;

		return true;
	}

	JPH::PhysicsSystem* Physics3D::s_PhysicsSystem;
	JPH::TempAllocator* Physics3D::s_TempAllocator;
	JPH::JobSystemThreadPool* Physics3D::s_JobSystem;

	BPLayerInterfaceImpl* Physics3D::s_BPLayerInterface;

	void Physics3D::Init()
	{
		ARC_PROFILE_SCOPE()

		JPH::RegisterDefaultAllocator();

		JPH::Factory::sInstance = new JPH::Factory();

		JPH::RegisterTypes();

		s_TempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
		s_JobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, static_cast<int>(JPH::thread::hardware_concurrency()) - 1);
		constexpr JPH::uint cMaxBodies = 65536;
		constexpr JPH::uint cNumBodyMutexes = 0;
		constexpr JPH::uint cMaxBodyPairs = 65536;
		constexpr JPH::uint cMaxContactConstraints = 10240;

		s_BPLayerInterface = new BPLayerInterfaceImpl();
		s_PhysicsSystem = new JPH::PhysicsSystem();
		s_PhysicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, *s_BPLayerInterface, Physics3DBroadPhaseCanCollide, Physics3DObjectCanCollide);
	}

	void Physics3D::Shutdown()
	{
		ARC_PROFILE_SCOPE()

		delete s_PhysicsSystem;
		delete s_BPLayerInterface;
		delete s_JobSystem;
		delete s_TempAllocator;
		delete JPH::Factory::sInstance;

		s_PhysicsSystem = nullptr;
		s_BPLayerInterface = nullptr;
		s_JobSystem = nullptr;
		s_TempAllocator = nullptr;
		JPH::Factory::sInstance = nullptr;
	}

	void Physics3D::Step(float physicsTs)
	{
		ARC_PROFILE_SCOPE()

		ARC_CORE_ASSERT(s_PhysicsSystem, "Physics system not initialized")

		s_PhysicsSystem->Update(physicsTs, 1, 1, s_TempAllocator, s_JobSystem);
	}

	JPH::PhysicsSystem& Physics3D::GetPhysicsSystem()
	{
		ARC_PROFILE_SCOPE()

		ARC_CORE_ASSERT(s_PhysicsSystem, "Physics system not initialized")

		return *s_PhysicsSystem;
	}
}
