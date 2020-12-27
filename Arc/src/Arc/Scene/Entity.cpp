#include "arcpch.h"
#include "Arc/Scene/Entity.h"

namespace ArcEngine
{
	Entity::Entity(entt::entity handle, Scene* scene)
		:m_EntityHandle(handle), m_Scene(scene)
	{
	}
}
