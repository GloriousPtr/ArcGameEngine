#pragma once

#include "Framebuffer.h"
#include "Shader.h"

namespace ArcEngine
{
	enum class CullModeType {	None = 0, Back, Front };
	enum class PrimitiveType { Triangle, Line, Point };
	enum class FillModeType { Solid, Wireframe };
	enum class DepthFuncType { None = 0, Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always };

	struct PipelineSpecification
	{
		CullModeType CullMode = CullModeType::Back;
		PrimitiveType Primitive = PrimitiveType::Triangle;
		FillModeType FillMode = FillModeType::Solid;
		bool EnableDepth = false;
		DepthFuncType DepthFunc = DepthFuncType::Less;
		std::vector<FramebufferTextureFormat> OutputFormats{};
	};

	class PipelineState
	{
	public:
		virtual ~PipelineState() = default;

		[[nodiscard]] virtual bool Bind() = 0;
		virtual bool Unbind() = 0;

		[[nodiscard]] virtual MaterialPropertyMap& GetMaterialProperties() = 0;
		[[nodiscard]] virtual uint32_t GetSlot(const std::string_view& name) = 0;

		template<typename T>
		void SetData(const std::string& name, const T& data, uint32_t size = 0, uint32_t offset = 0)
		{
			SetDataImpl(name, &data, (size == 0 ? sizeof(T) : size), offset);
		}

	private:
		virtual void SetDataImpl(const std::string& name, const void* data, uint32_t size, uint32_t offset) = 0;

	public:
		static Ref<PipelineState> Create(const Ref<Shader>& shader, const PipelineSpecification& spec);
	};
}
