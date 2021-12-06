#pragma once

#include "Arc/Renderer/Camera.h"
#include "Arc/Renderer/EditorCamera.h"
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
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static void DrawMesh(uint32_t entityID, Ref<VertexArray> vertexArrayList, Ref<Texture2D> texture, const glm::mat4& transform);
	};
}
