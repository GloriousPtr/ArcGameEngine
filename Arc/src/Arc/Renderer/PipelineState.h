#pragma once

#include "Framebuffer.h"
#include "Shader.h"

namespace ArcEngine
{
	enum class CullModeType {	None = 0, Back, Front };
	enum class PrimitiveType { Triangle, Line, Point };
	enum class FillModeType { Solid, Wireframe };
	enum class DepthFuncType { None = 0, Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always };

	struct GraphicsPipelineSpecification
	{
		CullModeType CullMode = CullModeType::Back;
		PrimitiveType Primitive = PrimitiveType::Triangle;
		FillModeType FillMode = FillModeType::Solid;
		bool EnableDepth = false;
		DepthFuncType DepthFunc = DepthFuncType::Less;
		FramebufferTextureFormat DepthFormat = FramebufferTextureFormat::Depth;
		eastl::vector<FramebufferTextureFormat> OutputFormats{};
	};

	struct PipelineSpecification
	{
		ShaderType Type = ShaderType::None;
		GraphicsPipelineSpecification GraphicsPipelineSpecs{};
	};

	enum class MaterialPropertyType
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
		MaterialPropertyType Type;
		uint32_t SizeInBytes;
		uint32_t StartOffsetInBytes;

		eastl::string Name;
		eastl::string DisplayName;
		bool IsSlider;
		bool IsColor;

		int32_t Slot = -1;

		[[nodiscard]] bool IsValid() const
		{
			return Slot != -1;
		}
	};

	class PipelineState
	{
	public:
		virtual ~PipelineState() = default;

		[[nodiscard]] virtual bool Bind() const = 0;
		virtual bool Unbind() const = 0;

		[[nodiscard]] virtual eastl::vector<MaterialProperty>& GetMaterialProperties() = 0;
		[[nodiscard]] virtual uint32_t GetSlot(const eastl::string_view& name) = 0;

		void SetData(const eastl::string_view name, const void* data, uint32_t size, uint32_t offset = 0)
		{
			SetDataImpl(name, data, size, offset);
		}

	private:
		virtual void SetDataImpl(const eastl::string_view name, const void* data, uint32_t size, uint32_t offset) = 0;

	public:
		static Ref<PipelineState> Create(const Ref<Shader>& shader, const PipelineSpecification& spec);
	};
}
