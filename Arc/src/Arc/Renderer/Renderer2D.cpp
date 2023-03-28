#include "arcpch.h"
#include "Arc/Renderer/Renderer2D.h"

#include <glm/gtc/matrix_transform.hpp>

#include "PipelineState.h"
#include "Renderer.h"
#include "Arc/Core/AssetManager.h"
#include "Arc/Renderer/VertexArray.h"
#include "Arc/Renderer/Shader.h"
#include "Arc/Renderer/RenderCommand.h"
#include "Arc/Renderer/RenderGraphData.h"
#include "Arc/Renderer/Texture.h"
#include "glm/gtc/type_ptr.hpp"

namespace ArcEngine
{
	struct QuadVertex
	{
		glm::vec4 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		glm::vec4 TilingOffset;
		uint32_t TexIndex;
	};

	struct LineVertex
	{
		glm::vec4 Position;
		glm::vec4 Color;
	};
	
	struct Renderer2DData
	{
		static constexpr uint32_t MaxQuads = 20000;
		static constexpr uint32_t MaxVertices = MaxQuads * 4;
		static constexpr uint32_t MaxIndices = MaxQuads * 6;
		static constexpr uint32_t MaxTextureSlots = 32;

		Ref<ConstantBuffer> CameraConstantBuffer;
		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<PipelineState> TexturePipeline;

		Ref<VertexBuffer> LineVertexBuffer;
		Ref<PipelineState> LinePipeline;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		uint32_t LineVertexCount = 0;
		LineVertex* LineVertexBufferBase = nullptr;
		LineVertex* LineVertexBufferPtr = nullptr;

		std::array<uint32_t, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = white texture

		static constexpr glm::vec4 QuadVertexPositions[4] =	  { { -0.5f, -0.5f, 0.0f, 1.0f },
																{  0.5f, -0.5f, 0.0f, 1.0f },
																{  0.5f,  0.5f, 0.0f, 1.0f },
																{ -0.5f,  0.5f, 0.0f, 1.0f } };

		static constexpr glm::vec2 TextureCoords[] = { { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f } };

		Renderer2D::Statistics Stats;
	};

	static Scope<Renderer2DData> s_Data;

	void Renderer2D::Init()
	{
		ARC_PROFILE_SCOPE()

		s_Data = CreateScope<Renderer2DData>();

		s_Data->QuadVertexArray = VertexArray::Create();
		
		s_Data->QuadVertexBuffer = VertexBuffer::Create(Renderer2DData::MaxVertices * sizeof(QuadVertex), sizeof(QuadVertex));
		s_Data->QuadVertexArray->AddVertexBuffer(s_Data->QuadVertexBuffer);

		s_Data->QuadVertexBufferBase = new QuadVertex[Renderer2DData::MaxVertices];

		const auto quadIndices = new uint32_t[Renderer2DData::MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < Renderer2DData::MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;
			
			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}
		
		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, Renderer2DData::MaxIndices);
		s_Data->QuadVertexArray->SetIndexBuffer(quadIB);
		delete[] quadIndices;

		// Lines
		{
			s_Data->LineVertexBuffer = VertexBuffer::Create(Renderer2DData::MaxVertices * sizeof(LineVertex), sizeof(LineVertex));
			s_Data->LineVertexBufferBase = new LineVertex[Renderer2DData::MaxVertices];
		}


		auto& pipelineLibrary = Renderer::GetPipelineLibrary();
		{
			const PipelineSpecification texture2dPippelineSpec
			{
				.Type = ShaderType::Pixel,
				.GraphicsPipelineSpecs
				{
					.CullMode = CullModeType::None,
					.Primitive = PrimitiveType::Triangle,
					.FillMode = FillModeType::Solid,
					.EnableDepth = false,
					.DepthFunc = DepthFuncType::Less,
					.OutputFormats = { FramebufferTextureFormat::R11G11B10F }
				}
			};
			s_Data->TexturePipeline = pipelineLibrary.Load("assets/shaders/Texture.hlsl", texture2dPippelineSpec);
		}

		s_Data->CameraConstantBuffer = ConstantBuffer::Create(sizeof(CameraData), 1, s_Data->TexturePipeline->GetSlot("GlobalData"));

		{
			const PipelineSpecification linePippelineSpec
			{
				.Type = ShaderType::Pixel,
				.GraphicsPipelineSpecs
				{
					.CullMode = CullModeType::None,
					.Primitive = PrimitiveType::Line,
					.FillMode = FillModeType::Solid,
					.EnableDepth = false,
					.DepthFunc = DepthFuncType::Less,
					.OutputFormats = { FramebufferTextureFormat::R11G11B10F }
				}
			};
			s_Data->LinePipeline = pipelineLibrary.Load("assets/shaders/Line.hlsl", linePippelineSpec);
		}

		s_Data->TextureSlots[0] = AssetManager::WhiteTexture()->GetIndex();
	}

	void Renderer2D::Shutdown()
	{
		ARC_PROFILE_SCOPE()

		delete[] s_Data->QuadVertexBufferBase;
		delete[] s_Data->LineVertexBufferBase;

		s_Data.reset();
	}

	void Renderer2D::BeginScene(const CameraData& viewProjection)
	{
		ARC_PROFILE_SCOPE()

		if (s_Data->TexturePipeline->Bind())
		{
			s_Data->CameraConstantBuffer->Bind(0);
			s_Data->CameraConstantBuffer->SetData(&viewProjection, sizeof(CameraData), 0);
		}

		StartBatch();
	}

	void Renderer2D::EndScene()
	{
		ARC_PROFILE_SCOPE()

		Flush();
	}

	void Renderer2D::StartBatch()
	{
		ARC_PROFILE_SCOPE()

		s_Data->QuadIndexCount = 0;
		s_Data->QuadVertexBufferPtr = s_Data->QuadVertexBufferBase;

		s_Data->LineVertexCount = 0;
		s_Data->LineVertexBufferPtr = s_Data->LineVertexBufferBase;

		s_Data->TextureSlotIndex = 1;
	}

	void Renderer2D::Flush()
	{
		ARC_PROFILE_SCOPE()

		if(s_Data->QuadIndexCount && s_Data->TexturePipeline->Bind())
		{
			ARC_PROFILE_SCOPE("Draw Quads")

			const auto dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(s_Data->QuadVertexBufferPtr) - reinterpret_cast<uint8_t*>(s_Data->QuadVertexBufferBase));
			s_Data->QuadVertexBuffer->SetData(s_Data->QuadVertexBufferBase, dataSize);
			s_Data->TexturePipeline->SetData("Textures", s_Data->TextureSlots.data(), sizeof(uint32_t) * s_Data->TextureSlotIndex);
			RenderCommand::DrawIndexed(s_Data->QuadVertexArray, s_Data->QuadIndexCount);
			s_Data->Stats.DrawCalls++;

			RenderCommand::Execute();
		}
		
		if (s_Data->LineVertexCount && s_Data->LinePipeline->Bind())
		{
			ARC_PROFILE_SCOPE("Draw Lines")

			const auto dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(s_Data->LineVertexBufferPtr) - reinterpret_cast<uint8_t*>(s_Data->LineVertexBufferBase));
			s_Data->LineVertexBuffer->SetData(s_Data->LineVertexBufferBase, dataSize);
			RenderCommand::DrawLines(s_Data->LineVertexBuffer, s_Data->LineVertexCount);
			s_Data->Stats.DrawCalls++;

			RenderCommand::Execute();
		}
	}

	void Renderer2D::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor, glm::vec2 tiling, glm::vec2 offset)
	{
		DrawQuad({ position.x, position.y, 0.0f }, rotation, size, texture, tintColor, tiling, offset);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor, glm::vec2 tiling, glm::vec2 offset)
	{
		ARC_PROFILE_SCOPE()
		
		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
								* glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f })
								* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, texture, tintColor, tiling, offset);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color)
	{
		ARC_PROFILE_SCOPE()
		
		constexpr size_t quadVertexCount = 4;
		constexpr uint32_t textureIndex = 0; // White Texture
		constexpr float tiling = 1.0f;
		constexpr float offset = 0.0f;
		
		if(s_Data->QuadIndexCount >= Renderer2DData::MaxIndices)
			NextBatch();

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data->QuadVertexBufferPtr->Position = transform * Renderer2DData::QuadVertexPositions[i];
			s_Data->QuadVertexBufferPtr->Color = color;
			s_Data->QuadVertexBufferPtr->TexCoord = Renderer2DData::TextureCoords[i];
			s_Data->QuadVertexBufferPtr->TilingOffset = glm::vec4(tiling, tiling, offset, offset);
			s_Data->QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data->QuadVertexBufferPtr++;
		}
		
		s_Data->QuadIndexCount += 6;

		s_Data->Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& tintColor, glm::vec2 tiling, glm::vec2 offset)
	{
		ARC_PROFILE_SCOPE()

		uint32_t textureIndex = 0;

		if(texture)
		{
			for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++)
			{
				if(s_Data->TextureSlots[i] == texture->GetIndex())
				{
					textureIndex = i;
					break;
				}
			}

			if(textureIndex == 0)
			{
				if(s_Data->QuadIndexCount >= Renderer2DData::MaxIndices || s_Data->TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
					NextBatch();
				
				textureIndex = s_Data->TextureSlotIndex;
				s_Data->TextureSlots[textureIndex] = texture->GetIndex();
				s_Data->TextureSlotIndex++;
			}
		}

		constexpr size_t quadVertexCount = 4;

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data->QuadVertexBufferPtr->Position = transform * Renderer2DData::QuadVertexPositions[i];
			s_Data->QuadVertexBufferPtr->Color = tintColor;
			s_Data->QuadVertexBufferPtr->TexCoord = Renderer2DData::TextureCoords[i];
			s_Data->QuadVertexBufferPtr->TilingOffset = glm::vec4(tiling, offset);
			s_Data->QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data->QuadVertexBufferPtr++;
		}
		
		s_Data->QuadIndexCount += 6;

		s_Data->Stats.QuadCount++;
	}

	void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
	{
		ARC_PROFILE_SCOPE()

		s_Data->LineVertexBufferPtr->Position = glm::vec4(p0, 1.0f);
		s_Data->LineVertexBufferPtr->Color = color;
		s_Data->LineVertexBufferPtr++;

		s_Data->LineVertexBufferPtr->Position = glm::vec4(p1, 1.0f);
		s_Data->LineVertexBufferPtr->Color = color;
		s_Data->LineVertexBufferPtr++;

		s_Data->LineVertexCount += 2;
	}

	void Renderer2D::DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		ARC_PROFILE_SCOPE()

		const glm::vec3 p0 = { position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z };
		const glm::vec3 p1 = { position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z };
		const glm::vec3 p2 = { position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z };
		const glm::vec3 p3 = { position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z };

		DrawLine(p0, p1, color);
		DrawLine(p1, p2, color);
		DrawLine(p2, p3, color);
		DrawLine(p3, p0, color);
	}

	void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color)
	{
		ARC_PROFILE_SCOPE()

		glm::vec3 lineVertices[4];
		for (size_t i = 0; i < 4; i++)
			lineVertices[i] = transform * Renderer2DData::QuadVertexPositions[i];

		DrawLine(lineVertices[0], lineVertices[1], color);
		DrawLine(lineVertices[1], lineVertices[2], color);
		DrawLine(lineVertices[2], lineVertices[3], color);
		DrawLine(lineVertices[3], lineVertices[0], color);
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_Data->Stats, 0, sizeof(Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_Data->Stats;
	}
}
