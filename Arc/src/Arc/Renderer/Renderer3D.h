#pragma once

#include "Arc/Renderer/Camera.h"
#include "Arc/Renderer/EditorCamera.h"
#include "Arc/Scene/Entity.h"
#include "Arc/Scene/Components.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

struct aiMesh;
struct aiScene;
struct aiNode;

namespace ArcEngine
{
	class Renderer3D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera, Entity cubemap, std::vector<Entity>& lights);
		static void EndScene(Ref<Framebuffer>& renderTarget);

		static void DrawCube();
		static void SubmitMesh(uint32_t entityID, MeshComponent& meshComponent, const glm::mat4& transform);

		static ShaderLibrary& GetShaderLibrary() { return s_ShaderLibrary; }
		static uint32_t GetShadowMapTextureID();

	private:
		static void SetupCameraData(const EditorCamera& camera);
		static void SetupLightsData();
		static void RenderPass(Ref<Framebuffer>& renderTarget);
		static void Flush(Ref<Framebuffer>& renderTarget);
		static void ShadowMapPass();

	private:
		static ShaderLibrary s_ShaderLibrary;
	};
}
