#include "arcpch.h"
#include "Arc/Scene/SceneSerializer.h"

#include "Arc/Scene/Entity.h"
#include "Arc/Scene/Components.h"

#include <fstream>

#include <yaml-cpp/yaml.h>

namespace YAML {

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

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		ARC_CORE_ASSERT(entity.HasComponent<IDComponent>());

		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

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
			out << YAML::Key << "TextureFilepath" << YAML::Value << spriteRendererComponent.TextureFilepath;
			out << YAML::Key << "TilingFactor" << YAML::Value << spriteRendererComponent.TilingFactor;

			out << YAML::EndMap; // SpriteRendererComponent
		}

		if (entity.HasComponent<SkyLightComponent>())
		{
			out << YAML::Key << "SkyLightComponent";
			out << YAML::BeginMap; // MeshComponent

			auto& skyLightComponent = entity.GetComponent<SkyLightComponent>();
			out << YAML::Key << "TextureFilepath" << YAML::Value << (skyLightComponent.Texture ? skyLightComponent.Texture->GetPath() : "");
			out << YAML::Key << "Intensity" << YAML::Value << skyLightComponent.Intensity;

			out << YAML::EndMap; // SkyLightComponent
		}

		if (entity.HasComponent<LightComponent>())
		{
			out << YAML::Key << "LightComponent";
			out << YAML::BeginMap; // LightComponent

			auto& lightComponent = entity.GetComponent<LightComponent>();
			out << YAML::Key << "Type" << YAML::Value << (int) lightComponent.Type;
			out << YAML::Key << "Color" << YAML::Value << lightComponent.Color;
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
			out << YAML::Key << "Type" << YAML::Value << (int) rb2d.Type;
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

		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.each([&](auto entityID)
		{
			Entity entity = { entityID, m_Scene.get() };
			if (!entity)
				return;

			SerializeEntity(out, entity);
		});
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		// Not implemented
		ARC_CORE_ASSERT(false);
	}

#define TryGet(x, outType, defValue) ((x) ? x.as<outType>() : defValue)

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		ARC_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				ARC_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

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
					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)TryGet(cameraProps["ProjectionType"], int, 0));

					cc.Camera.SetPerspectiveVerticalFOV(TryGet(cameraProps["PerspectiveFOV"], float, 45.0f));
					cc.Camera.SetPerspectiveNearClip(TryGet(cameraProps["PerspectiveNear"], float, 0.03f));
					cc.Camera.SetPerspectiveFarClip(TryGet(cameraProps["PerspectiveFar"], float, 1000.0f));

					cc.Camera.SetOrthographicSize(TryGet(cameraProps["OrthographicSize"], float, 10.0f));
					cc.Camera.SetOrthographicNearClip(TryGet(cameraProps["OrthographicNear"], float, -1.0f));
					cc.Camera.SetOrthographicFarClip(TryGet(cameraProps["OrthographicFar"], float, 1.0f));

					cc.Primary = TryGet(cameraComponent["Primary"], bool, true);
					cc.FixedAspectRatio = TryGet(cameraComponent["FixedAspectRatio"], bool, false);
				}

				auto spriteRendererComponent = entity["SpriteRendererComponent"];
				if (spriteRendererComponent)
				{
					auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
					src.Color = TryGet(spriteRendererComponent["Color"], glm::vec4, glm::vec4(1.0f));
					std::string textureFilepath = TryGet(spriteRendererComponent["TextureFilepath"], std::string, "");
					if(!textureFilepath.empty())
						src.SetTexture(textureFilepath);
					src.TilingFactor = TryGet(spriteRendererComponent["TilingFactor"], float, 1.0f);
				}

				auto skyLightComponent = entity["SkyLightComponent"];
				if (skyLightComponent)
				{
					auto& src = deserializedEntity.AddComponent<SkyLightComponent>();
					std::string textureFilepath = TryGet(skyLightComponent["TextureFilepath"], std::string, "");
					if(!textureFilepath.empty())
						src.SetTexture(textureFilepath);
					src.Intensity = TryGet(skyLightComponent["Intensity"], float, 1.0f);
				}

				auto lightComponent = entity["LightComponent"];
				if (lightComponent)
				{
					auto& src = deserializedEntity.AddComponent<LightComponent>();
					src.Type = (LightComponent::LightType) TryGet(lightComponent["Type"], int, 1);
					src.Color = TryGet(lightComponent["Color"], glm::vec3, glm::vec3(1.0f));
					src.Intensity = TryGet(lightComponent["Intensity"], float, 10.0f);
					src.Range = TryGet(lightComponent["Radius"], float, 1.0f);
					src.CutOffAngle = TryGet(lightComponent["CutOffAngle"], float, 12.5f);
					src.OuterCutOffAngle = TryGet(lightComponent["OuterCutOffAngle"], float, 17.5f);
				}

				auto rb2dCpmponent = entity["Rigidbody2DComponent"];
				if (rb2dCpmponent)
				{
					auto& src = deserializedEntity.AddComponent<Rigidbody2DComponent>();
					src.Type = (Rigidbody2DComponent::BodyType) TryGet(rb2dCpmponent["Type"], int, 0);
					src.AutoMass = TryGet(rb2dCpmponent["AutoMass"], bool, true);
					src.Mass = TryGet(rb2dCpmponent["Mass"], float, 1.0f);
					src.LinearDrag = TryGet(rb2dCpmponent["LinearDrag"], float, 0.0f);
					src.AngularDrag = TryGet(rb2dCpmponent["AngularDrag"], float, 0.05f);
					src.AllowSleep = TryGet(rb2dCpmponent["AllowSleep"], bool, true);
					src.Awake = TryGet(rb2dCpmponent["Awake"], bool, true);
					src.Continuous = TryGet(rb2dCpmponent["Continuous"], bool, false);
					src.FreezeRotation = TryGet(rb2dCpmponent["FreezeRotation"], bool, false);
					src.GravityScale = TryGet(rb2dCpmponent["GravityScale"], float, 1.0f);
				}

				auto bc2dCpmponent = entity["BoxCollider2DComponent"];
				if (bc2dCpmponent)
				{
					auto& src = deserializedEntity.AddComponent<BoxCollider2DComponent>();
					src.Size = TryGet(bc2dCpmponent["Size"], glm::vec2, glm::vec2(0.5f));
					src.Offset = TryGet(bc2dCpmponent["Offset"], glm::vec2, glm::vec2(0.0f));
					src.Density = TryGet(bc2dCpmponent["Density"], float, 1.0f);
					src.Friction = TryGet(bc2dCpmponent["Friction"], float, 0.5f);
					src.Restitution = TryGet(bc2dCpmponent["Restitution"], float, 0.0f);
					src.RestitutionThreshold = TryGet(bc2dCpmponent["RestitutionThreshold"], float, 0.5f);
				}
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		// Not implemented
		ARC_CORE_ASSERT(false);
		return false;
	}
}
