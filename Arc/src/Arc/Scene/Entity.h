#pragma once

#include "Arc/Core/UUID.h"
#include "Arc/Scene/Components.h"
#include "Arc/Scene/Scene.h"

#include "entt.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <optick.h>

namespace ArcEngine
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			OPTICK_EVENT();

			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			OPTICK_EVENT();

			ARC_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent()
		{
			OPTICK_EVENT();

			return m_Scene->m_Registry.has<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			OPTICK_EVENT();

			ARC_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		UUID GetUUID() { return GetComponent<IDComponent>(); }
		TransformComponent& GetTransform() { return GetComponent<TransformComponent>(); }
		
		Entity GetParent()
		{
			OPTICK_EVENT();

			ARC_PROFILE_FUNCTION();

			auto& transform = GetTransform();
			return transform.Parent != 0 ? m_Scene->GetEntity(transform.Parent) : Entity {};
		}

		void SetParent(Entity parent)
		{
			OPTICK_EVENT();

			ARC_PROFILE_FUNCTION();

			ARC_ASSERT(m_Scene->m_EntityMap.find(parent.GetUUID()) != m_Scene->m_EntityMap.end(), "Parent is not in the same scene as entity");
			Deparent();
			
			auto& transform = GetTransform();
			transform.Parent = parent.GetUUID();
			parent.GetTransform().Children.push_back(GetUUID());
		}

		void Deparent()
		{
			OPTICK_EVENT();

			ARC_PROFILE_FUNCTION();

			auto& transform = GetTransform();
			if (transform.Parent == 0)
				return;

			UUID uuid = GetUUID();
			auto& parent = GetParent().GetTransform();
			for (auto it = parent.Children.begin(); it != parent.Children.end(); it++)
			{
				if (*it == uuid)
				{
					parent.Children.erase(it);
					break;
				}
			}
			transform.Parent = 0;
		}

		
		glm::mat4 GetWorldTransform()
		{
			OPTICK_EVENT();

			ARC_PROFILE_FUNCTION();

			auto& transform = GetTransform();
			glm::mat4 parentTransform = glm::mat4(1.0f);
			if (transform.Parent != 0)
				parentTransform = m_Scene->GetEntity(transform.Parent).GetWorldTransform();

			return parentTransform * glm::translate(glm::mat4(1.0f), transform.Translation) * glm::toMat4(glm::quat(transform.Rotation)) * glm::scale(glm::mat4(1.0f), transform.Scale);
		}

		glm::mat4 GetLocalTransform()
		{
			OPTICK_EVENT();

			ARC_PROFILE_FUNCTION();

			auto& transform = GetTransform();
			return glm::translate(glm::mat4(1.0f), transform.Translation) * glm::toMat4(glm::quat(transform.Rotation)) * glm::scale(glm::mat4(1.0f), transform.Scale);
		}

		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
		bool operator!=(const Entity& other) const { return !(*this == other); }
	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	};
}
