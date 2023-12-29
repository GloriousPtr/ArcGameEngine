#pragma once

#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>

namespace ArcEngine
{
	class PhysicsMaterial3D : public JPH::PhysicsMaterialSimple
	{
	public:
		PhysicsMaterial3D(const eastl::string_view inName, JPH::ColorArg inColor, float inFriction, float inRestitution)
			: JPH::PhysicsMaterialSimple(inName.data(), inColor), Friction(inFriction), Restitution(inRestitution) { }

		float Friction;
		float Restitution;
	};
}
