#pragma once

namespace ArcEngine
{
	struct CameraData;
	class Texture2D;
	class Framebuffer;

	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();
		
		static void BeginScene(const CameraData& viewProjection, Ref<Framebuffer>& renderTarget, bool clearRenderTarget);
		static void EndScene();
		static void Flush();

		//Primitives
		static void DrawQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture = nullptr, const glm::vec4& tintColor = glm::vec4(1.0f), glm::vec2 tiling = glm::vec2(1.0f), glm::vec2 offset = glm::vec2(0.0f));
		static void DrawQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture = nullptr, const glm::vec4& tintColor = glm::vec4(1.0f), glm::vec2 tiling = glm::vec2(1.0f), glm::vec2 offset = glm::vec2(0.0f));

		static void DrawQuad(glm::mat4&& transform, const glm::vec4& color);
		static void DrawQuad(glm::mat4&& transform, const Ref<Texture2D>& texture = nullptr, const glm::vec4& tintColor = glm::vec4(1.0f), glm::vec2 tiling = glm::vec2(1.0f), glm::vec2 offset = glm::vec2(0.0f));

		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color);

		static void DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawRect(glm::mat4&& transform, const glm::vec4& color);

		// Stats
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			[[nodiscard]] uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
			[[nodiscard]] uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
			[[nodiscard]] uint32_t GetTotalTriangleCount() const { return QuadCount * 2; }
		};

		static void ResetStats();
		[[nodiscard]] static Statistics GetStats();
	private:
		static void StartBatch();
		static void NextBatch();
	};
}
