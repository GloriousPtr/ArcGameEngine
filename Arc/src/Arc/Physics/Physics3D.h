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
		struct BroadLayer
		{
			static constexpr uint8_t STATIC  = 0;
			static constexpr uint8_t DEFAULT = 1;
			static constexpr uint8_t OTHER2  = 2;
			static constexpr uint8_t OTHER3  = 3;
			static constexpr uint8_t OTHER4  = 4;
			static constexpr uint8_t OTHER5  = 5;
			static constexpr uint8_t OTHER6  = 6;
			static constexpr uint8_t OTHER7  = 7;
			static constexpr uint8_t OTHER8  = 8;
			static constexpr uint8_t OTHER9  = 9;
			static constexpr uint8_t OTHER10 = 10;
			static constexpr uint8_t OTHER11 = 11;
			static constexpr uint8_t OTHER12 = 12;
			static constexpr uint8_t OTHER13 = 13;
			static constexpr uint8_t OTHER14 = 14;
			static constexpr uint8_t OTHER15 = 15;

			static constexpr uint8_t NUM_LAYERS = 16;
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
