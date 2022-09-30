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

	class Physics3D
	{
	public:
		static void Init();
		static void Shutdown();
		static void Step(float physicsTs);

		static JPH::PhysicsSystem& GetPhysicsSystem();

	private:
		static JPH::PhysicsSystem* s_PhysicsSystem;
		static JPH::TempAllocator* s_TempAllocator;
		static JPH::JobSystemThreadPool* s_JobSystem;
		static BPLayerInterfaceImpl* s_BPLayerInterface;
	};
}
