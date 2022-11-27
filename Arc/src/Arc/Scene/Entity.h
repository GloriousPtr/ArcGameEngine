#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "Arc/Core/UUID.h"
#include "Arc/Debug/Profiler.h"
#include "Arc/Scene/Components.h"
#include "Arc/Scene/Scene.h"

namespace ArcEngine
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) const
		{
			ARC_PROFILE_SCOPE();

			ARC_CORE_ASSERT(m_Scene, "Scene is null!");
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, eastl::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		T& GetComponent() const
		{
			ARC_PROFILE_SCOPE();

			ARC_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent() const
		{
			ARC_PROFILE_SCOPE();

			ARC_CORE_ASSERT(m_Scene, "Scene is null!");
			return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent() const
		{
			ARC_PROFILE_SCOPE();

			ARC_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		UUID GetUUID() const { return GetComponent<IDComponent>().ID; }
		eastl::string_view GetTag() const { return GetComponent<TagComponent>().Tag; }
		TransformComponent& GetTransform() const { return GetComponent<TransformComponent>(); }
		RelationshipComponent& GetRelationship() const { return GetComponent<RelationshipComponent>(); }
		
		Entity GetParent() const
		{
			ARC_PROFILE_SCOPE();

			if (!m_Scene)
				return {};

			const auto& rc = GetComponent<RelationshipComponent>();
			return rc.Parent != 0 ? m_Scene->GetEntity(rc.Parent) : Entity {};
		}

		void SetParent(Entity parent) const
		{
			ARC_PROFILE_SCOPE();

			ARC_CORE_ASSERT(m_Scene->m_EntityMap.find(parent.GetUUID()) != m_Scene->m_EntityMap.end(), "Parent is not in the same scene as entity");
			Deparent();
			
			auto& rc = GetComponent<RelationshipComponent>();
			rc.Parent = parent.GetUUID();
			parent.GetRelationship().Children.emplace_back(GetUUID());
		}

		void Deparent() const
		{
			ARC_PROFILE_SCOPE();

			auto& transform = GetRelationship();
			UUID uuid = GetUUID();
			Entity parentEntity = GetParent();
			
			if (!parentEntity)
				return;

			auto& parent = parentEntity.GetRelationship();
			for (const auto* it = parent.Children.begin(); it != parent.Children.end(); it++)
			{
				if (*it == uuid)
				{
					parent.Children.erase(it);
					break;
				}
			}
			transform.Parent = 0;
		}
		
		glm::mat4 GetWorldTransform() const
		{
			ARC_PROFILE_SCOPE();

			const auto& transform = GetTransform();
			const auto& rc = GetRelationship();
			Entity parent = m_Scene->GetEntity(rc.Parent);
			glm::mat4 parentTransform = parent ? parent.GetWorldTransform() : glm::mat4(1.0f);
			return parentTransform * glm::translate(glm::mat4(1.0f), transform.Translation) * glm::toMat4(glm::quat(transform.Rotation)) * glm::scale(glm::mat4(1.0f), transform.Scale);
		}

		glm::mat4 GetLocalTransform() const
		{
			ARC_PROFILE_SCOPE();

			const auto& transform = GetTransform();
			return glm::translate(glm::mat4(1.0f), transform.Translation) * glm::toMat4(glm::quat(transform.Rotation)) * glm::scale(glm::mat4(1.0f), transform.Scale);
		}

		Scene* GetScene() const { return m_Scene; }

		operator bool() const { return m_EntityHandle != entt::null && m_Scene != nullptr; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
		bool operator!=(const Entity& other) const { return !(*this == other); }
	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	};
}
