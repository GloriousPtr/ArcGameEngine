#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>

namespace ArcEngine
{
	class PhysicsMaterial3D : public JPH::PhysicsMaterialSimple
	{
	public:
		PhysicsMaterial3D(const std::string& inName, JPH::ColorArg inColor, float inFriction, float inRestitution)
			: JPH::PhysicsMaterialSimple(inName, inColor), Friction(inFriction), Restitution(inRestitution) { }

		float Friction;
		float Restitution;
	};
}
