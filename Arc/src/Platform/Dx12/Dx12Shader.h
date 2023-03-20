#pragma once

#include "Arc/Core/Base.h"
#include "Arc/Renderer/Shader.h"
#include "Platform/Dx12/Dx12Resources.h"

#include <glm/glm.hpp>

namespace ArcEngine
{
	class Dx12Shader : public Shader
	{
	public:
		explicit Dx12Shader(const std::filesystem::path& filepath);
		~Dx12Shader() override;

		Dx12Shader(const Dx12Shader& other) = default;
		Dx12Shader(Dx12Shader&& other) = default;

		void Recompile(const std::filesystem::path& path) override;
		void Bind() const override;
		void Unbind() const override;

		[[nodiscard]] MaterialPropertyMap& GetMaterialProperties() override;
		[[nodiscard]] const std::string& GetName() const override;

	private:
		void SetDataImpl(const std::string& name, const void* value, uint32_t size, uint32_t offset) override;
		void Compile(const std::filesystem::path& filepath);

	private:
		std::string																			m_Name;
		ID3D12RootSignature*																m_RootSignature = nullptr;
		ID3D12PipelineState*																m_PipelineState = nullptr;
		std::unordered_map<std::string, MaterialProperty, UM_StringTransparentEquality>		m_MaterialProperties;
	};
}
