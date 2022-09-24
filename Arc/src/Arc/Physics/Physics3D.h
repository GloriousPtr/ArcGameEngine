#pragma once

namespace JPH
{
	class BodyInterface;
}

namespace ArcEngine
{
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
		static void Step(float ts);

		static JPH::BodyInterface* GetBodyInterface();
		static void OptimizeBroadPhase();
	};
}
