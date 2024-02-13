#include "arcpch.h"
#include "Arc/Renderer/Renderer2D.h"

#include "ParticleSystem.h"
#include "PipelineState.h"
#include "Renderer.h"
#include "Arc/Core/AssetManager.h"
#include "Arc/Renderer/VertexArray.h"
#include "Arc/Renderer/Shader.h"
#include "Arc/Renderer/RenderCommand.h"
#include "Arc/Renderer/RenderGraphData.h"
#include "Arc/Renderer/Texture.h"

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

		Ref<Framebuffer> RenderTarget = nullptr;
		GraphicsCommandList CommandList = nullptr;

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

		eastl::array<uint32_t, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = white texture

		static constexpr glm::vec4 QuadVertexPositions[4] =	  { { -0.5f, -0.5f, 0.0f, 1.0f },
																{  0.5f, -0.5f, 0.0f, 1.0f },
																{  0.5f,  0.5f, 0.0f, 1.0f },
																{ -0.5f,  0.5f, 0.0f, 1.0f } };

		static constexpr glm::vec2 TextureCoords[] = { { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f } };

		Renderer2D::Statistics Stats;
	};

	static Scope<Renderer2DData> s_Renderer2DData;

	void Renderer2D::Init()
	{
		ARC_PROFILE_SCOPE();

		s_Renderer2DData = CreateScope<Renderer2DData>();

		s_Renderer2DData->QuadVertexArray = VertexArray::Create();
		
		s_Renderer2DData->QuadVertexBuffer = VertexBuffer::Create(Renderer2DData::MaxVertices * sizeof(QuadVertex), sizeof(QuadVertex));
		s_Renderer2DData->QuadVertexArray->AddVertexBuffer(s_Renderer2DData->QuadVertexBuffer);

		s_Renderer2DData->QuadVertexBufferBase = new QuadVertex[Renderer2DData::MaxVertices];

		uint32_t* quadIndices = new uint32_t[Renderer2DData::MaxIndices];

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
		s_Renderer2DData->QuadVertexArray->SetIndexBuffer(quadIB);
		delete[] quadIndices;

		// Lines
		{
			s_Renderer2DData->LineVertexBuffer = VertexBuffer::Create(Renderer2DData::MaxVertices * sizeof(LineVertex), sizeof(LineVertex));
			s_Renderer2DData->LineVertexBufferBase = new LineVertex[Renderer2DData::MaxVertices];
		}


		PipelineLibrary& pipelineLibrary = Renderer::GetPipelineLibrary();
		{
			const PipelineSpecification texture2dPippelineSpec
			{
				.Type = ShaderType::Pixel,
				.GraphicsPipelineSpecs
				{
					.CullMode = CullModeType::None,
					.Primitive = PrimitiveType::Triangle,
					.FillMode = FillModeType::Solid,
					.DepthFormat = FramebufferTextureFormat::None,
					.OutputFormats = { FramebufferTextureFormat::R11G11B10F }
				}
			};
			s_Renderer2DData->TexturePipeline = pipelineLibrary.Load("assets/shaders/Texture.hlsl", texture2dPippelineSpec);
		}

		s_Renderer2DData->TexturePipeline->RegisterCB(CRC32("GlobalData"), sizeof(CameraData));

		{
			const PipelineSpecification linePippelineSpec
			{
				.Type = ShaderType::Pixel,
				.GraphicsPipelineSpecs
				{
					.CullMode = CullModeType::None,
					.Primitive = PrimitiveType::Line,
					.FillMode = FillModeType::Solid,
					.DepthFormat = FramebufferTextureFormat::None,
					.OutputFormats = { FramebufferTextureFormat::R11G11B10F }
				}
			};
			s_Renderer2DData->LinePipeline = pipelineLibrary.Load("assets/shaders/Line.hlsl", linePippelineSpec);
		}

		s_Renderer2DData->TextureSlots[0] = AssetManager::WhiteTexture()->GetHeapIndex();
	}

	void Renderer2D::Shutdown()
	{
		ARC_PROFILE_SCOPE();

		delete[] s_Renderer2DData->QuadVertexBufferBase;
		delete[] s_Renderer2DData->LineVertexBufferBase;

		s_Renderer2DData.reset();
	}

	void Renderer2D::BeginScene(const CameraData& viewProjection, Ref<Framebuffer>& renderTarget, bool clearRenderTarget)
	{
		ARC_PROFILE_SCOPE();

		s_Renderer2DData->RenderTarget = renderTarget;

		s_Renderer2DData->CommandList = RenderCommand::BeginRecordingCommandList();
		if (clearRenderTarget)
			s_Renderer2DData->RenderTarget->Clear(s_Renderer2DData->CommandList);
		if (s_Renderer2DData->TexturePipeline->Bind(s_Renderer2DData->CommandList))
			s_Renderer2DData->TexturePipeline->SetCBData(s_Renderer2DData->CommandList, CRC32("GlobalData"), &viewProjection, sizeof(CameraData));

		StartBatch();
	}

	void Renderer2D::EndScene()
	{
		ARC_PROFILE_SCOPE();

		Flush();
		RenderCommand::EndRecordingCommandList(s_Renderer2DData->CommandList);
	}

	void Renderer2D::StartBatch()
	{
		ARC_PROFILE_SCOPE();

		s_Renderer2DData->QuadIndexCount = 0;
		s_Renderer2DData->QuadVertexBufferPtr = s_Renderer2DData->QuadVertexBufferBase;

		s_Renderer2DData->LineVertexCount = 0;
		s_Renderer2DData->LineVertexBufferPtr = s_Renderer2DData->LineVertexBufferBase;

		s_Renderer2DData->TextureSlotIndex = 1;
	}

	void Renderer2D::Flush()
	{
		ARC_PROFILE_SCOPE();

		if (s_Renderer2DData->RenderTarget)
		{
			GraphicsCommandList commandList = s_Renderer2DData->CommandList;
			s_Renderer2DData->RenderTarget->Bind(commandList);

			if (s_Renderer2DData->QuadIndexCount)
			{
				if (s_Renderer2DData->TexturePipeline->Bind(commandList))
				{
					ARC_PROFILE_SCOPE_NAME("Draw Quads");

					const uint32_t dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(s_Renderer2DData->QuadVertexBufferPtr) - reinterpret_cast<uint8_t*>(s_Renderer2DData->QuadVertexBufferBase));
					s_Renderer2DData->TexturePipeline->BindCB(commandList, CRC32("GlobalData"));
					s_Renderer2DData->QuadVertexBuffer->SetData(commandList, s_Renderer2DData->QuadVertexBufferBase, dataSize);
					s_Renderer2DData->TexturePipeline->SetRSData(commandList, CRC32(BindlessTexturesSlotName), s_Renderer2DData->TextureSlots.data(), sizeof(uint32_t) * s_Renderer2DData->TextureSlotIndex);
					RenderCommand::DrawIndexed(commandList, s_Renderer2DData->QuadVertexArray, s_Renderer2DData->QuadIndexCount);
					s_Renderer2DData->Stats.DrawCalls++;
				}
			}

			if (s_Renderer2DData->LineVertexCount)
			{
				if (s_Renderer2DData->LinePipeline->Bind(commandList))
				{
					ARC_PROFILE_SCOPE_NAME("Draw Lines");

					const uint32_t dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(s_Renderer2DData->LineVertexBufferPtr) - reinterpret_cast<uint8_t*>(s_Renderer2DData->LineVertexBufferBase));
					s_Renderer2DData->LineVertexBuffer->SetData(commandList, s_Renderer2DData->LineVertexBufferBase, dataSize);
					RenderCommand::DrawLines(commandList, s_Renderer2DData->LineVertexBuffer, s_Renderer2DData->LineVertexCount);
					s_Renderer2DData->Stats.DrawCalls++;
				}
			}
		}
	}

	void Renderer2D::NextBatch()
	{
		Flush();
		RenderCommand::EndRecordingCommandList(s_Renderer2DData->CommandList, true);
		s_Renderer2DData->CommandList = RenderCommand::BeginRecordingCommandList();
		StartBatch();
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor, glm::vec2 tiling, glm::vec2 offset)
	{
		DrawQuad({ position.x, position.y, 0.0f }, rotation, size, texture, tintColor, tiling, offset);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor, glm::vec2 tiling, glm::vec2 offset)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
								* glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f })
								* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(eastl::move(transform), texture, tintColor, tiling, offset);
	}

	void Renderer2D::DrawQuad(glm::mat4&& transform, const glm::vec4& color)
	{
		ARC_PROFILE_SCOPE();
		
		constexpr size_t quadVertexCount = 4;
		constexpr uint32_t textureIndex = 0; // White Texture
		constexpr float tiling = 1.0f;
		constexpr float offset = 0.0f;
		
		if(s_Renderer2DData->QuadIndexCount >= Renderer2DData::MaxIndices)
			NextBatch();

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Renderer2DData->QuadVertexBufferPtr->Position = transform * Renderer2DData::QuadVertexPositions[i];
			s_Renderer2DData->QuadVertexBufferPtr->Color = color;
			s_Renderer2DData->QuadVertexBufferPtr->TexCoord = Renderer2DData::TextureCoords[i];
			s_Renderer2DData->QuadVertexBufferPtr->TilingOffset = glm::vec4(tiling, tiling, offset, offset);
			s_Renderer2DData->QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Renderer2DData->QuadVertexBufferPtr++;
		}
		
		s_Renderer2DData->QuadIndexCount += 6;

		s_Renderer2DData->Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(glm::mat4&& transform, const Ref<Texture2D>& texture, const glm::vec4& tintColor, glm::vec2 tiling, glm::vec2 offset)
	{
		ARC_PROFILE_SCOPE();

		if (s_Renderer2DData->QuadIndexCount >= Renderer2DData::MaxIndices)
			NextBatch();

		uint32_t textureIndex = 0;

		if(texture)
		{
			for (uint32_t i = 1; i < s_Renderer2DData->TextureSlotIndex; i++)
			{
				if(s_Renderer2DData->TextureSlots[i] == texture->GetHeapIndex())
				{
					textureIndex = i;
					break;
				}
			}

			if(textureIndex == 0)
			{
				if(s_Renderer2DData->TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
					NextBatch();
				
				textureIndex = s_Renderer2DData->TextureSlotIndex;
				s_Renderer2DData->TextureSlots[textureIndex] = texture->GetHeapIndex();
				s_Renderer2DData->TextureSlotIndex++;
			}
		}

		constexpr size_t quadVertexCount = 4;

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Renderer2DData->QuadVertexBufferPtr->Position = transform * Renderer2DData::QuadVertexPositions[i];
			s_Renderer2DData->QuadVertexBufferPtr->Color = tintColor;
			s_Renderer2DData->QuadVertexBufferPtr->TexCoord = Renderer2DData::TextureCoords[i];
			s_Renderer2DData->QuadVertexBufferPtr->TilingOffset = glm::vec4(tiling, offset);
			s_Renderer2DData->QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Renderer2DData->QuadVertexBufferPtr++;
		}
		
		s_Renderer2DData->QuadIndexCount += 6;

		s_Renderer2DData->Stats.QuadCount++;
	}

	void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
	{
		ARC_PROFILE_SCOPE();

		if (s_Renderer2DData->LineVertexCount >= Renderer2DData::MaxVertices)
			NextBatch();

		s_Renderer2DData->LineVertexBufferPtr->Position = glm::vec4(p0, 1.0f);
		s_Renderer2DData->LineVertexBufferPtr->Color = color;
		s_Renderer2DData->LineVertexBufferPtr++;

		s_Renderer2DData->LineVertexBufferPtr->Position = glm::vec4(p1, 1.0f);
		s_Renderer2DData->LineVertexBufferPtr->Color = color;
		s_Renderer2DData->LineVertexBufferPtr++;

		s_Renderer2DData->LineVertexCount += 2;
	}

	void Renderer2D::DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		ARC_PROFILE_SCOPE();

		const glm::vec3 p0 = { position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z };
		const glm::vec3 p1 = { position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z };
		const glm::vec3 p2 = { position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z };
		const glm::vec3 p3 = { position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z };

		DrawLine(p0, p1, color);
		DrawLine(p1, p2, color);
		DrawLine(p2, p3, color);
		DrawLine(p3, p0, color);
	}

	void Renderer2D::DrawRect(glm::mat4&& transform, const glm::vec4& color)
	{
		ARC_PROFILE_SCOPE();

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
		memset(&s_Renderer2DData->Stats, 0, sizeof(Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_Renderer2DData->Stats;
	}
}
