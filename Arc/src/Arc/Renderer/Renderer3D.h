#pragma once

#include "Arc/Renderer/Camera.h"
#include "Arc/Renderer/EditorCamera.h"
#include "Arc/Scene/Entity.h"
#include "Arc/Scene/Components.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "RenderGraphData.h"

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
		static void EndScene(Ref<RenderGraphData>& renderTarget);

		static void DrawCube();
		static void DrawQuad();
		static void SubmitMesh(MeshComponent& meshComponent, const glm::mat4& transform);

		static ShaderLibrary& GetShaderLibrary() { return s_ShaderLibrary; }

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t IndexCount = 0;
		};

		static void ResetStats();
		static Statistics GetStats();

	private:
		static void SetupCameraData(const EditorCamera& camera);
		static void SetupLightsData();
		static void Flush(Ref<RenderGraphData> renderGraphData);
		static void CompositePass(Ref<RenderGraphData> renderGraphData);
		static void BloomPass(Ref<RenderGraphData> renderGraphData);
		static void RenderPass(Ref<Framebuffer> renderTarget);
		static void ShadowMapPass();

	private:
		static ShaderLibrary s_ShaderLibrary;

	public:
		enum class TonemappingType { None = 0, ACES, Filmic, Uncharted };

		static TonemappingType Tonemapping;
		static float Exposure;
		static bool UseBloom;
		static float BloomStrength;
		static float BloomThreshold;
		static float BloomKnee;
		static float BloomClamp;
	};
}
