#pragma once

#include "Framebuffer.h"
#include "Shader.h"

namespace ArcEngine
{
	constexpr const char* MaterialPropertiesSlotName = "MaterialProperties";
	constexpr const char* BindlessTexturesSlotName = "Textures";

	enum class CullModeType : uint8_t { None = 0, Back, Front };
	enum class PrimitiveType : uint8_t { Triangle, Line, Point };
	enum class FillModeType : uint8_t { Solid, Wireframe };
	enum class DepthFuncType : uint8_t { None = 0, Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always };

	struct GraphicsPipelineSpecification
	{
		CullModeType CullMode = CullModeType::Back;
		PrimitiveType Primitive = PrimitiveType::Triangle;
		FillModeType FillMode = FillModeType::Solid;
		DepthFuncType DepthFunc = DepthFuncType::Less;
		FramebufferTextureFormat DepthFormat = FramebufferTextureFormat::Depth;
		eastl::vector<FramebufferTextureFormat> OutputFormats{};
	};

	struct PipelineSpecification
	{
		ShaderType Type = ShaderType::None;
		GraphicsPipelineSpecification GraphicsPipelineSpecs{};
	};

	enum class MaterialPropertyType : uint8_t
	{
		None = 0,
		Texture2D,
		Texture2DBindless,
		Bool,
		Int,
		UInt,
		Float,
		Float2,
		Float3,
		Float4,
	};

	struct MaterialProperty
	{
		eastl::string Name;
		eastl::string DisplayName;
		uint32_t SizeInBytes;
		uint32_t StartOffsetInBytes;

		int32_t Slot = -1;
		bool IsSlider;
		bool IsColor;

		MaterialPropertyType Type;

		[[nodiscard]] bool IsValid() const
		{
			return Slot != -1;
		}
	};

	class PipelineState
	{
	public:
		virtual ~PipelineState() = default;

		virtual void Bind(GraphicsCommandList commandList) const = 0;
		virtual void Unbind(GraphicsCommandList commandList) const = 0;

		[[nodiscard]] virtual eastl::vector<MaterialProperty>& GetMaterialProperties() = 0;
		[[nodiscard]] virtual uint32_t GetSlot(const eastl::string_view name) = 0;

		virtual void RegisterCB(eastl::string_view name, uint32_t size) = 0;
		virtual void RegisterSB(eastl::string_view name, uint32_t stride, uint32_t count) = 0;

		virtual void BindCB(GraphicsCommandList commandList, uint32_t crc) = 0;
		virtual void BindSB(GraphicsCommandList commandList, uint32_t crc) = 0;

		virtual void SetRSData(GraphicsCommandList commandList, eastl::string_view name, const void* data, uint32_t size, uint32_t offset = 0) = 0;
		virtual void SetCBData(GraphicsCommandList commandList, uint32_t crc, const void* data, uint32_t size, uint32_t offset = 0) = 0;
		virtual void SetSBData(GraphicsCommandList commandList, uint32_t crc, const void* data, uint32_t size, uint32_t index) = 0;

	public:
		static Ref<PipelineState> Create(const Ref<Shader>& shader, const PipelineSpecification& spec);
	};
}
