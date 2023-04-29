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

		[[nodiscard]] static JPH::PhysicsSystem& GetPhysicsSystem();
	};
}
