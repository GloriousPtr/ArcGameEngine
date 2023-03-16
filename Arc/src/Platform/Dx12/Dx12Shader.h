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
		Dx12Shader(const std::filesystem::path& filepath, const BufferLayout& layout);
		~Dx12Shader() override;

		Dx12Shader(const Dx12Shader& other) = default;
		Dx12Shader(Dx12Shader&& other) = default;

		void Recompile(const std::filesystem::path& path) override;
		void Bind() const override;
		void Unbind() const override;

		void SetInt(const std::string& name, int value) override;
		void SetIntArray(const std::string& name, const int* values, uint32_t count) override;
		void SetFloat(const std::string& name, float value) override;
		void SetFloat2(const std::string& name, const glm::vec2& value) override;
		void SetFloat3(const std::string& name, const glm::vec3& value) override;
		void SetFloat4(const std::string& name, const glm::vec4& value) override;
		void SetMat3(const std::string& name, const glm::mat3& value) override;
		void SetMat4(const std::string& name, const glm::mat4& value) override;
		void SetData(uint32_t slot, uint32_t num32BitValues, void* data) override;

		[[nodiscard]] std::unordered_map<std::string, MaterialProperty, UM_StringTransparentEquality>& GetMaterialProperties() override;

		[[nodiscard]] const std::string& GetName() const override;

	private:
		void SetInputLayoutFromBufferLayout(const BufferLayout& layout);
		void Compile(const std::filesystem::path& filepath);

	private:
		std::string									m_Name;
		ID3D12RootSignature*						m_RootSignature = nullptr;
		ID3D12PipelineState*						m_PipelineState = nullptr;
	};
}
