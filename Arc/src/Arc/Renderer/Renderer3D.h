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
		static void DrawQuad();
		static void SubmitMesh(MeshComponent& meshComponent, const glm::mat4& transform);

		static ShaderLibrary& GetShaderLibrary() { return s_ShaderLibrary; }

	private:
		static void SetupCameraData(const EditorCamera& camera);
		static void SetupLightsData();
		static void Flush(Ref<Framebuffer>& renderTarget);
		static void CompositePass(Ref<Framebuffer>& renderTarget);
		static void BloomPass();
		static void RenderPass();
		static void ShadowMapPass();

	private:
		static ShaderLibrary s_ShaderLibrary;

	public:
		enum class TonemappingType { None = 0, ACES, Filmic, Uncharted };

		static const int blurSamples = 6;
		static Ref<Framebuffer> prefilteredFramebuffer;
		static Ref<Framebuffer> tempBlurFramebuffers[blurSamples];
		static Ref<Framebuffer> downsampledFramebuffers[blurSamples];
		static Ref<Framebuffer> upsampledFramebuffers[blurSamples];

		static TonemappingType Tonemapping;
		static float Exposure;
		static bool UseBloom;
		static float BloomStrength;
		static float BloomThreshold;
		static float BloomKnee;
		static float BloomClamp;
	};
}
