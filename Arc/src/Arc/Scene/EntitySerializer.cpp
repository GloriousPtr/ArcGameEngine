#include "arcpch.h"
#include "EntitySerializer.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

#include "Arc/Core/AssetManager.h"
#include "Arc/Utils/StringUtils.h"

namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<eastl::string>
	{
		static Node encode(const eastl::string& str)
		{
			Node node;
			node.push_back(str.c_str());
			return node;
		}

		static bool decode(const Node& node, eastl::string& str)
		{
			str = node.as<std::string>().c_str();
			return true;
		}
	};
}

namespace ArcEngine
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

#define TryGet(node, defaultValue) ((node) ? node.as<decltype(defaultValue)>() : defaultValue)
#define TryGetEnum(node, defaultValue) ((node) ? (decltype(defaultValue)) node.as<int>() : defaultValue)

	void EntitySerializer::SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		ARC_CORE_ASSERT(entity.HasComponent<IDComponent>());

		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag.c_str();

			out << YAML::EndMap; // TagComponent
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent

			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			out << YAML::EndMap; // TransformComponent
		}

		if (entity.HasComponent<RelationshipComponent>())
		{
			out << YAML::Key << "RelationshipComponent";
			out << YAML::BeginMap; // RelationshipComponent

			auto& tc = entity.GetComponent<RelationshipComponent>();
			out << YAML::Key << "Parent" << YAML::Value << tc.Parent;

			out << YAML::Key << "ChildrenCount" << YAML::Value << tc.Children.size();
			out << YAML::Key << "Children";
			out << YAML::BeginMap; // Children
			for (size_t i = 0; i < tc.Children.size(); i++)
				out << YAML::Key << i << YAML::Value << tc.Children[i];
			out << YAML::EndMap; // Children

			out << YAML::EndMap; // RelationshipComponent
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Camera
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap; // Camera

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap; // CameraComponent
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent

			auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;
			out << YAML::Key << "TexturePath" << YAML::Value << (spriteRendererComponent.Texture ? spriteRendererComponent.Texture->GetPath().c_str() : "");
			out << YAML::Key << "TilingFactor" << YAML::Value << spriteRendererComponent.TilingFactor;

			out << YAML::EndMap; // SpriteRendererComponent
		}

		if (entity.HasComponent<SkyLightComponent>())
		{
			out << YAML::Key << "SkyLightComponent";
			out << YAML::BeginMap; // MeshComponent

			auto& skyLightComponent = entity.GetComponent<SkyLightComponent>();
			out << YAML::Key << "TexturePath" << YAML::Value << (skyLightComponent.Texture ? skyLightComponent.Texture->GetPath().c_str() : "");
			out << YAML::Key << "Intensity" << YAML::Value << skyLightComponent.Intensity;

			out << YAML::EndMap; // SkyLightComponent
		}

		if (entity.HasComponent<LightComponent>())
		{
			out << YAML::Key << "LightComponent";
			out << YAML::BeginMap; // LightComponent

			auto& lightComponent = entity.GetComponent<LightComponent>();
			out << YAML::Key << "Type" << YAML::Value << (int)lightComponent.Type;
			out << YAML::Key << "ShadowQuality" << YAML::Value << (int)lightComponent.ShadowQuality;
			out << YAML::Key << "Color" << YAML::Value << lightComponent.Color;
			out << YAML::Key << "UseColorTempratureMode" << YAML::Value << lightComponent.UseColorTempratureMode;
			out << YAML::Key << "Intensity" << YAML::Value << lightComponent.Intensity;
			out << YAML::Key << "Radius" << YAML::Value << lightComponent.Range;
			out << YAML::Key << "CutOffAngle" << YAML::Value << lightComponent.CutOffAngle;
			out << YAML::Key << "OuterCutOffAngle" << YAML::Value << lightComponent.OuterCutOffAngle;

			out << YAML::EndMap; // LightComponent
		}

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Rigidbody2DComponent

			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			out << YAML::Key << "Type" << YAML::Value << (int)rb2d.Type;
			out << YAML::Key << "AutoMass" << YAML::Value << rb2d.AutoMass;
			out << YAML::Key << "Mass" << YAML::Value << rb2d.Mass;
			out << YAML::Key << "LinearDrag" << YAML::Value << rb2d.LinearDrag;
			out << YAML::Key << "AngularDrag" << YAML::Value << rb2d.AngularDrag;
			out << YAML::Key << "AllowSleep" << YAML::Value << rb2d.AllowSleep;
			out << YAML::Key << "Awake" << YAML::Value << rb2d.Awake;
			out << YAML::Key << "Continuous" << YAML::Value << rb2d.Continuous;
			out << YAML::Key << "FreezeRotation" << YAML::Value << rb2d.FreezeRotation;
			out << YAML::Key << "GravityScale" << YAML::Value << rb2d.GravityScale;

			out << YAML::EndMap; // Rigidbody2DComponent
		}

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // BoxCollider2DComponent

			auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Size" << YAML::Value << bc2d.Size;
			out << YAML::Key << "Offset" << YAML::Value << bc2d.Offset;
			out << YAML::Key << "Density" << YAML::Value << bc2d.Density;
			out << YAML::Key << "Friction" << YAML::Value << bc2d.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << bc2d.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2d.RestitutionThreshold;

			out << YAML::EndMap; // BoxCollider2DComponent
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; // CircleCollider2DComponent

			auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
			out << YAML::Key << "Radius" << YAML::Value << cc2d.Radius;
			out << YAML::Key << "Offset" << YAML::Value << cc2d.Offset;
			out << YAML::Key << "Density" << YAML::Value << cc2d.Density;
			out << YAML::Key << "Friction" << YAML::Value << cc2d.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << cc2d.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2d.RestitutionThreshold;

			out << YAML::EndMap; // CircleCollider2DComponent
		}

		out << YAML::EndMap; // Entity
	}

	UUID EntitySerializer::DeserializeEntity(YAML::Node node, Scene& scene, bool preserveUUID)
	{
		YAML::Node entity = node;

		uint64_t uuid = entity["Entity"].as<uint64_t>();

		eastl::string name;
		auto tagComponent = entity["TagComponent"];
		if (tagComponent)
			name = tagComponent["Tag"].as<std::string>().c_str();

		Entity deserializedEntity;
		if (preserveUUID)
			deserializedEntity = scene.CreateEntityWithUUID(uuid, name);
		else
			deserializedEntity = scene.CreateEntity(name);

		if (preserveUUID)
			ARC_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);
		else
			ARC_CORE_TRACE("Deserialized entity with oldID = {0}, newID = {1}, name = {2}", uuid, (uint64_t)deserializedEntity.GetUUID(), name);

		auto transformComponent = entity["TransformComponent"];
		if (transformComponent)
		{
			// Entities always have transforms
			auto& tc = deserializedEntity.GetComponent<TransformComponent>();
			tc.Translation = transformComponent["Translation"].as<glm::vec3>();
			tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
			tc.Scale = transformComponent["Scale"].as<glm::vec3>();
		}

		auto relationshipComponent = entity["RelationshipComponent"];
		if (relationshipComponent)
		{
			// Entities always have transforms
			auto& tc = deserializedEntity.GetComponent<RelationshipComponent>();
			tc.Parent = relationshipComponent["Parent"].as<uint64_t>();

			size_t childCount = relationshipComponent["ChildrenCount"].as<size_t>();
			tc.Children.clear();
			tc.Children.reserve(childCount);
			auto children = relationshipComponent["Children"];
			if (children && childCount > 0)
			{
				for (size_t i = 0; i < childCount; i++)
					tc.Children.push_back(children[i].as<uint64_t>());
			}
		}

		auto cameraComponent = entity["CameraComponent"];
		if (cameraComponent)
		{
			auto& cc = deserializedEntity.AddComponent<CameraComponent>();

			auto& cameraProps = cameraComponent["Camera"];
			cc.Camera.SetViewportSize(1, 1);
			cc.Camera.SetProjectionType(TryGetEnum(cameraProps["ProjectionType"], SceneCamera::ProjectionType::Perspective));

			cc.Camera.SetPerspectiveVerticalFOV(TryGet(cameraProps["PerspectiveFOV"], 45.0f));
			cc.Camera.SetPerspectiveNearClip(TryGet(cameraProps["PerspectiveNear"], 0.03f));
			cc.Camera.SetPerspectiveFarClip(TryGet(cameraProps["PerspectiveFar"], 1000.0f));

			cc.Camera.SetOrthographicSize(TryGet(cameraProps["OrthographicSize"], 10.0f));
			cc.Camera.SetOrthographicNearClip(TryGet(cameraProps["OrthographicNear"], -1.0f));
			cc.Camera.SetOrthographicFarClip(TryGet(cameraProps["OrthographicFar"], 1.0f));

			cc.Primary = TryGet(cameraComponent["Primary"], true);
			cc.FixedAspectRatio = TryGet(cameraComponent["FixedAspectRatio"], false);
		}

		auto spriteRenderer = entity["SpriteRendererComponent"];
		if (spriteRenderer)
		{
			auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
			src.Color = TryGet(spriteRenderer["Color"], glm::vec4(1.0f));
			eastl::string texturePath = TryGet(spriteRenderer["TexturePath"], eastl::string(""));
			if (!texturePath.empty())
				src.Texture = AssetManager::GetTexture2D(texturePath);
			src.TilingFactor = TryGet(spriteRenderer["TilingFactor"], 1.0f);
		}

		auto skyLight = entity["SkyLightComponent"];
		if (skyLight)
		{
			auto& src = deserializedEntity.AddComponent<SkyLightComponent>();
			eastl::string texturePath = TryGet(skyLight["TexturePath"], eastl::string(""));
			if (!texturePath.empty())
				src.Texture = AssetManager::GetTextureCubemap(texturePath);
			src.Intensity = TryGet(skyLight["Intensity"], 1.0f);
		}

		auto lightComponent = entity["LightComponent"];
		if (lightComponent)
		{
			auto& src = deserializedEntity.AddComponent<LightComponent>();
			src.Type = TryGetEnum(lightComponent["Type"], LightComponent::LightType::Point);
			src.ShadowQuality = TryGetEnum(lightComponent["ShadowQuality"], LightComponent::ShadowQualityType::UltraSoft);
			src.Color = TryGet(lightComponent["Color"], glm::vec3(1.0f));
			src.UseColorTempratureMode = TryGet(lightComponent["UseColorTempratureMode"], false);
			src.Intensity = TryGet(lightComponent["Intensity"], 10.0f);
			src.Range = TryGet(lightComponent["Radius"], 1.0f);
			src.CutOffAngle = TryGet(lightComponent["CutOffAngle"], 12.5f);
			src.OuterCutOffAngle = TryGet(lightComponent["OuterCutOffAngle"], 17.5f);
		}

		auto rb2dCpmponent = entity["Rigidbody2DComponent"];
		if (rb2dCpmponent)
		{
			auto& src = deserializedEntity.AddComponent<Rigidbody2DComponent>();
			src.Type = TryGetEnum(rb2dCpmponent["Type"], Rigidbody2DComponent::BodyType::Static);
			src.AutoMass = TryGet(rb2dCpmponent["AutoMass"], true);
			src.Mass = TryGet(rb2dCpmponent["Mass"], 1.0f);
			src.LinearDrag = TryGet(rb2dCpmponent["LinearDrag"], 0.0f);
			src.AngularDrag = TryGet(rb2dCpmponent["AngularDrag"], 0.05f);
			src.AllowSleep = TryGet(rb2dCpmponent["AllowSleep"], true);
			src.Awake = TryGet(rb2dCpmponent["Awake"], true);
			src.Continuous = TryGet(rb2dCpmponent["Continuous"], false);
			src.FreezeRotation = TryGet(rb2dCpmponent["FreezeRotation"], false);
			src.GravityScale = TryGet(rb2dCpmponent["GravityScale"], 1.0f);
		}

		auto bc2dCpmponent = entity["BoxCollider2DComponent"];
		if (bc2dCpmponent)
		{
			auto& src = deserializedEntity.AddComponent<BoxCollider2DComponent>();
			src.Size = TryGet(bc2dCpmponent["Size"], glm::vec2(0.5f));
			src.Offset = TryGet(bc2dCpmponent["Offset"], glm::vec2(0.0f));
			src.Density = TryGet(bc2dCpmponent["Density"], 1.0f);
			src.Friction = TryGet(bc2dCpmponent["Friction"], 0.5f);
			src.Restitution = TryGet(bc2dCpmponent["Restitution"], 0.0f);
			src.RestitutionThreshold = TryGet(bc2dCpmponent["RestitutionThreshold"], 0.5f);
		}

		auto cc2dCpmponent = entity["CircleCollider2DComponent"];
		if (cc2dCpmponent)
		{
			auto& src = deserializedEntity.AddComponent<CircleCollider2DComponent>();
			src.Radius = TryGet(cc2dCpmponent["Radius"], 0.5f);
			src.Offset = TryGet(cc2dCpmponent["Offset"], glm::vec2(0.0f));
			src.Density = TryGet(cc2dCpmponent["Density"], 1.0f);
			src.Friction = TryGet(cc2dCpmponent["Friction"], 0.5f);
			src.Restitution = TryGet(cc2dCpmponent["Restitution"], 0.0f);
			src.RestitutionThreshold = TryGet(cc2dCpmponent["RestitutionThreshold"], 0.5f);
		}

		return deserializedEntity.GetUUID();
	}

	static void GetAllChildren(Entity parent, eastl::vector<Entity>& outEntities)
	{
		eastl::vector<UUID> children = parent.GetComponent<RelationshipComponent>().Children;
		for (auto& child : children)
		{
			Entity e = parent.GetScene()->GetEntity(child);
			outEntities.push_back(e);
			GetAllChildren(e, outEntities);
		}
	}

	void EntitySerializer::SerializeEntityAsPrefab(const char* filepath, Entity entity)
	{
		if (entity.HasComponent<PrefabComponent>())
		{
			ARC_CORE_ERROR("Entity already has a prefab component!");
			return;
		}

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Prefab" << YAML::Value << (uint64_t)entity.AddComponent<PrefabComponent>().ID;
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		eastl::vector<Entity> entities;
		entities.push_back(entity);
		GetAllChildren(entity, entities);

		for (auto& entity : entities)
		{
			if (!entity)
				return;

			EntitySerializer::SerializeEntity(out, entity);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	bool EntitySerializer::DeserializeEntityAsPrefab(const char* filepath, Scene& scene)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Prefab"])
			return false;

		uint64_t prefabID = data["Prefab"].as<uint64_t>();

		auto entities = data["Entities"];
		ARC_CORE_TRACE("Deserializing prefab : {0} ({1})", StringUtils::GetName(filepath).c_str(), prefabID);

		if (entities)
		{
			bool first = true;
			eastl::unordered_map<UUID, UUID> oldNewIdMap;
			for (auto entity : entities)
			{
				UUID oldUUID = entity["Entity"].as<uint64_t>();
				UUID newUUID = EntitySerializer::DeserializeEntity(entity, scene, false);
				oldNewIdMap.emplace(oldUUID, newUUID);
				
				if (first)
				{
					first = false;
					scene.GetEntity(newUUID).AddComponent<PrefabComponent>().ID = prefabID;
				}
			}

			// Fix parent/children UUIDs
			for (auto [oldId, newId] : oldNewIdMap)
			{
				auto& relationshipComponent = scene.GetEntity(newId).GetRelationship();
				UUID parent = relationshipComponent.Parent;
				if (parent)
					relationshipComponent.Parent = oldNewIdMap.at(parent);

				auto& children = relationshipComponent.Children;
				for (size_t i = 0; i < children.size(); i++)
				{
					UUID childId = children[i];
					children[i] = oldNewIdMap.at(childId);
				}
			}
		}
		else
		{
			ARC_CORE_ERROR("There are no entities in the prefab {0} ({1}) to deserialize!", StringUtils::GetName(filepath).c_str(), prefabID);
		}

		return true;
	}
}
