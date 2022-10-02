#pragma once

#include <EASTL/unordered_map.h>

#include "Arc/Renderer/Shader.h"

namespace ArcEngine
{
	class OpenGLShader : public Shader
	{
	public:
		explicit OpenGLShader(const eastl::string& filepath);
		OpenGLShader(const eastl::string& name, const eastl::string& vertexSrc, const eastl::string& fragmentSrc);
		virtual ~OpenGLShader();

		OpenGLShader(const OpenGLShader& other) = default;
		OpenGLShader(OpenGLShader&& other) = default;

		virtual void Recompile(const eastl::string& filepath);

		virtual void Bind() const;
		virtual void Unbind() const;

		virtual void SetInt(const eastl::string& name, int value) override;
		virtual void SetIntArray(const eastl::string& name, const int* values, uint32_t count) override;
		virtual void SetFloat(const eastl::string& name, float value) override;
		virtual void SetFloat2(const eastl::string& name, const glm::vec2& value) override;
		virtual void SetFloat3(const eastl::string& name, const glm::vec3& value) override;
		virtual void SetFloat4(const eastl::string& name, const glm::vec4& value) override;
		virtual void SetMat3(const eastl::string& name, const glm::mat3& value) override;
		virtual void SetMat4(const eastl::string& name, const glm::mat4& value) override;
		virtual void SetUniformBlock(const eastl::string& name, uint32_t blockIndex) override;

		virtual eastl::hash_map<eastl::string, MaterialProperty>& GetMaterialProperties() { return m_MaterialProperties; }

		virtual const eastl::string& GetName() const override { return m_Name; }
		
		void UploadUniformInt(const eastl::string& name, int value);
		void UploadUniformIntArray(const eastl::string& name, const int* values, uint32_t count);

		void UploadUniformFloat(const eastl::string& name, float value);
		void UploadUniformFloat2(const eastl::string& name, const glm::vec2& values);
		void UploadUniformFloat3(const eastl::string& name, const glm::vec3& values);
		void UploadUniformFloat4(const eastl::string& name, const glm::vec4& values);
		
		void UploadUniformMat3(const eastl::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const eastl::string& name, const glm::mat4& matrix);

	private:
		int GetLocation(const eastl::string& name);
		eastl::string ReadFile(const eastl::string& filepath) const;
		eastl::unordered_map<uint32_t, eastl::string> PreProcess(const eastl::string& source) const;
		void Compile(const eastl::unordered_map<uint32_t, eastl::string>& shaderSources);

	private:
		uint32_t m_RendererID;
		eastl::string m_Name;
		eastl::hash_map<eastl::string, int> m_UniformLocationCache;
		eastl::hash_map<eastl::string, MaterialProperty> m_MaterialProperties;
	};
}
