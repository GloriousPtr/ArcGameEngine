#pragma once

#include "Arc/Core/Base.h"
#include "Arc/Core/Timestep.h"

#include <box2d/b2_world.h>

#include <glm/glm.hpp>

namespace ArcEngine
{
	class Physics2D
	{
	public:
		static void Init();

		static void OnUpdate();

		static Ref<b2World> const GetWorld() { return s_World; }
	public:
		static glm::vec2 Gravity;
		static float Timestep;
		static int VelocityIterations;
		static int PositionIterations;
	private:
		static Ref<b2World> s_World;
	};
}

