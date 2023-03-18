#pragma once

#include "Arc/Renderer/Shader.h"

namespace ArcEngine
{
	class OpenGLShader : public Shader
	{
	public:
		explicit OpenGLShader(const std::filesystem::path& filepath);
		~OpenGLShader() override;

		OpenGLShader(const OpenGLShader& other) = default;
		OpenGLShader(OpenGLShader&& other) = default;

		void Recompile(const std::filesystem::path& filepath) override;

		void Bind() const override;
		void Unbind() const override;

		void SetInt(const std::string& name, int value, uint32_t offset = 0) override;
		void SetUInt(const std::string& name, unsigned int value, uint32_t offset = 0) override;
		void SetIntArray(const std::string& name, const int* values, uint32_t count, uint32_t offset = 0) override;
		void SetFloat(const std::string& name, float value, uint32_t offset = 0) override;
		void SetFloat2(const std::string& name, const glm::vec2& value, uint32_t offset = 0) override;
		void SetFloat3(const std::string& name, const glm::vec3& value, uint32_t offset = 0) override;
		void SetFloat4(const std::string& name, const glm::vec4& value, uint32_t offset = 0) override;
		void SetMat3(const std::string& name, const glm::mat3& value, uint32_t offset = 0) override;
		void SetMat4(const std::string& name, const glm::mat4& value, uint32_t offset = 0) override;

		[[nodiscard]] std::unordered_map<std::string, MaterialProperty, UM_StringTransparentEquality>& GetMaterialProperties() override { return m_MaterialProperties; }

		[[nodiscard]] const std::string& GetName() const override { return m_Name; }
		
		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformIntArray(const std::string& name, const int* values, uint32_t count);

		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& values);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& values);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& values);
		
		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

	private:
		[[nodiscard]] int GetLocation(const std::string& name);
		[[nodiscard]] std::unordered_map<uint32_t, std::string> PreProcess(std::string_view source) const;
		void Compile(const std::unordered_map<uint32_t, std::string>& shaderSources);
		
	private:
		uint32_t m_RendererID = 0;
		std::string m_Name;
		std::unordered_map<std::string, int, UM_StringTransparentEquality> m_UniformLocationCache;
		std::unordered_map<std::string, MaterialProperty, UM_StringTransparentEquality> m_MaterialProperties;
	};
}
