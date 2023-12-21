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

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) const
		{
			ARC_PROFILE_SCOPE();

			ARC_CORE_ASSERT(m_Scene, "Scene is null!");
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		[[nodiscard]] T& GetComponent() const
		{
			ARC_PROFILE_SCOPE();

			ARC_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		[[nodiscard]] bool HasComponent() const
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

		template<typename... Components>
		[[nodiscard]] auto GetAllComponents() const
		{
			ARC_PROFILE_SCOPE();

			return m_Scene->m_Registry.get<Components...>(m_EntityHandle);
		}

		[[nodiscard]] UUID GetUUID() const { return GetComponent<IDComponent>().ID; }
		[[nodiscard]] eastl::string_view GetTag() const { return GetComponent<TagComponent>().Tag; }
		[[nodiscard]] TransformComponent& GetTransform() const { return GetComponent<TransformComponent>(); }
		[[nodiscard]] RelationshipComponent& GetRelationship() const { return GetComponent<RelationshipComponent>(); }
		
		[[nodiscard]] Entity GetParent() const
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
			const UUID uuid = GetUUID();
			const Entity parentEntity = GetParent();
			
			if (!parentEntity)
				return;

			auto& parent = parentEntity.GetRelationship();
			for (auto it = parent.Children.begin(); it != parent.Children.end(); ++it)
			{
				if (*it == uuid)
				{
					parent.Children.erase(it);
					break;
				}
			}
			transform.Parent = 0;
		}
		
		[[nodiscard]] glm::mat4 GetWorldTransform() const
		{
			ARC_PROFILE_SCOPE();

			const auto& transform = GetTransform();
			const auto& rc = GetRelationship();
			const Entity parent = m_Scene->GetEntity(rc.Parent);
			const glm::mat4 parentTransform = parent ? parent.GetWorldTransform() : glm::mat4(1.0f);
			return eastl::move(parentTransform * glm::translate(glm::mat4(1.0f), transform.Translation) * glm::toMat4(glm::quat(transform.Rotation)) * glm::scale(glm::mat4(1.0f), transform.Scale));
		}

		[[nodiscard]] glm::mat4 GetLocalTransform() const
		{
			ARC_PROFILE_SCOPE();

			const auto& transform = GetTransform();
			return eastl::move(glm::translate(glm::mat4(1.0f), transform.Translation) * glm::toMat4(glm::quat(transform.Rotation)) * glm::scale(glm::mat4(1.0f), transform.Scale));
		}

		[[nodiscard]] Scene* GetScene() const { return m_Scene; }

		operator bool() const { return m_EntityHandle != entt::null && m_Scene != nullptr && m_Scene->m_Registry.valid(m_EntityHandle); }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return static_cast<uint32_t>(m_EntityHandle); }

		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }

	private:
		entt::entity m_EntityHandle = entt::null;
		Scene* m_Scene = nullptr;
	};
}
