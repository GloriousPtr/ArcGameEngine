#pragma once

namespace JPH
{
	class BodyInterface;
	class PhysicsSystem;
	class TempAllocator;
	class JobSystemThreadPool;
}

namespace ArcEngine
{
	class BPLayerInterfaceImpl;
	class Physics3DContactListener;
	class Physics3DBodyActivationListener;

	class Physics3D
	{
	public:
		struct Layers
		{
			static constexpr uint8_t NON_MOVING = 0;
			static constexpr uint8_t MOVING = 1;
			static constexpr uint8_t NUM_LAYERS = 2;
		};

		static void Init();
		static void Shutdown();
		static void Step(float physicsTs);

		static JPH::BodyInterface* GetBodyInterface();
		static void OptimizeBroadPhase();

	private:
		static JPH::PhysicsSystem* s_PhysicsSystem;
		static JPH::TempAllocator* s_TempAllocator;
		static JPH::JobSystemThreadPool* s_JobSystem;

		static BPLayerInterfaceImpl* s_BPLayerInterface;
		static Physics3DContactListener* s_ContactListener;
		static Physics3DBodyActivationListener* s_BodyActivationListener;
	};
}
