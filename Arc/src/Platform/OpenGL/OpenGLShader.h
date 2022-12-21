#pragma once

#include "Arc/Renderer/Shader.h"

namespace ArcEngine
{
	class OpenGLShader : public Shader
	{
	public:
		explicit OpenGLShader(const std::string& filepath);
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		~OpenGLShader() override;

		OpenGLShader(const OpenGLShader& other) = default;
		OpenGLShader(OpenGLShader&& other) = default;

		void Recompile(const std::string& filepath) override;

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
		void SetUniformBlock(const std::string& name, uint32_t blockIndex) override;

		std::unordered_map<std::string, MaterialProperty>& GetMaterialProperties() override { return m_MaterialProperties; }

		const std::string& GetName() const override { return m_Name; }
		
		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformIntArray(const std::string& name, const int* values, uint32_t count);

		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& values);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& values);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& values);
		
		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

	private:
		int GetLocation(const std::string& name);
		std::unordered_map<uint32_t, std::string> PreProcess(const std::string& source) const;
		void Compile(const std::unordered_map<uint32_t, std::string>& shaderSources);

	private:
		uint32_t m_RendererID;
		std::string m_Name;
		std::unordered_map<std::string, int> m_UniformLocationCache;
		std::unordered_map<std::string, MaterialProperty> m_MaterialProperties;
	};
}
