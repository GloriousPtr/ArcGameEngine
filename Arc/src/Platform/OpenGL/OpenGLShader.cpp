#include "arcpch.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "Arc/Core/Filesystem.h"

typedef uint32_t GLenum;

namespace ArcEngine
{
	static GLenum ShaderTypeFromString(std::string_view type)
	{
		if(type == "vertex")
			return GL_VERTEX_SHADER;
		if(type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;
		if(type == "compute")
			return GL_COMPUTE_SHADER;

		ARC_CORE_ASSERT(false, "Unknown shader type!")
		return 0;
	}
	
	OpenGLShader::OpenGLShader(const std::filesystem::path& filepath)
	{
		ARC_PROFILE_SCOPE()

		const std::string source = Filesystem::ReadFileText(filepath);
		const auto shaderSources = PreProcess(source);
		Compile(shaderSources);
		m_Name = filepath.filename().string();
	}

	OpenGLShader::~OpenGLShader()
	{
		ARC_PROFILE_SCOPE()
		
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Recompile(const std::filesystem::path& filepath)
	{
		ARC_PROFILE_SCOPE()

		glDeleteProgram(m_RendererID);

		const std::string source = Filesystem::ReadFileText(filepath);
		const auto shaderSources = PreProcess(source);
		Compile(shaderSources);
	}

	void OpenGLShader::Bind() const
	{
		ARC_PROFILE_SCOPE()
		
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		ARC_PROFILE_SCOPE()
		
		glUseProgram(0);
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		ARC_PROFILE_SCOPE()
		
		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, const int* values, uint32_t count)
	{
		ARC_PROFILE_SCOPE()
		
		UploadUniformIntArray(name, values, count);		
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		ARC_PROFILE_SCOPE()
		
		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		ARC_PROFILE_SCOPE()
		
		UploadUniformFloat2(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		ARC_PROFILE_SCOPE()
		
		UploadUniformFloat3(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		ARC_PROFILE_SCOPE()
		
		UploadUniformFloat4(name, value);
	}

	void OpenGLShader::SetMat3(const std::string& name, const glm::mat3& value)
	{
		ARC_PROFILE_SCOPE()
		
		UploadUniformMat3(name, value);
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		ARC_PROFILE_SCOPE()
		
		UploadUniformMat4(name, value);
	}

	void OpenGLShader::SetUniformBlock(const std::string& name, uint32_t blockIndex)
	{
		ARC_PROFILE_SCOPE()

		glUniformBlockBinding(m_RendererID, glGetUniformBlockIndex(m_RendererID, name.c_str()), blockIndex);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		glUniform1i(GetLocation(name), value);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, const int* values, uint32_t count)
	{
		glUniform1iv(GetLocation(name), static_cast<int>(count), values);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		glUniform1f(GetLocation(name), value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& values)
	{
		glUniform2f(GetLocation(name), values.x, values.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& values)
	{
		glUniform3f(GetLocation(name), values.x, values.y, values.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& values)
	{
		glUniform4f(GetLocation(name), values.x, values.y, values.z, values.w);
	}
	
	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		glUniformMatrix3fv(GetLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		glUniformMatrix4fv(GetLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	int OpenGLShader::GetLocation(const std::string& name)
	{
		ARC_PROFILE_SCOPE()

		const auto& it = m_UniformLocationCache.find(name);
		if (it != m_UniformLocationCache.end())
			return it->second;

		int location = glGetUniformLocation(m_RendererID, name.c_str());
		m_UniformLocationCache.emplace(name, location);
		return location;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(std::string_view source) const
	{
		ARC_PROFILE_SCOPE()
		
		std::unordered_map<GLenum, std::string> shaderSources;

		constexpr const char* typeToken = "#type";
		constexpr size_t typeTokenLength = std::string_view(typeToken).size();
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			const size_t eol = source.find_first_of("\r\n", pos);
			ARC_CORE_ASSERT(eol != std::string::npos, "Syntax error")
			const size_t begin = pos + typeTokenLength + 1;
			const std::string type = static_cast<std::string>(source.substr(begin, eol - begin));
			ARC_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified")

			const size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			shaderSources[ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	static MaterialPropertyType GetMaterialPropertyType(GLenum property)
	{
		switch (property)
		{
			case GL_SAMPLER_2D:		return MaterialPropertyType::Sampler2D;
			case GL_BOOL:			return MaterialPropertyType::Bool;
			case GL_INT:			return MaterialPropertyType::Int;
			case GL_FLOAT:			return MaterialPropertyType::Float;
			case GL_FLOAT_VEC2:		return MaterialPropertyType::Float2;
			case GL_FLOAT_VEC3:		return MaterialPropertyType::Float3;
			case GL_FLOAT_VEC4:		return MaterialPropertyType::Float4;
			default:				return MaterialPropertyType::None;
		}
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		ARC_PROFILE_SCOPE()

		const GLuint program = glCreateProgram();
		ARC_CORE_ASSERT(shaderSources.size() <= 2, "We only support 2 shaders for now")
		std::array<GLenum, 2> glShaderIDs = {};
		int glShaderIDIndex = 0;
		for (const auto& [type, source] : shaderSources)
		{
			const GLuint shader = glCreateShader(type);
			
			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, nullptr);

			// Compile the vertex shader
			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if(isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog.data());
				
				glDeleteShader(shader);

				ARC_CORE_ERROR("{0}", infoLog.data());
				ARC_CORE_ASSERT(false, "Shader compilation failure!")
				break;
			}

			glAttachShader(program, shader);
			glShaderIDs[glShaderIDIndex] = shader;
			++glShaderIDIndex;
		}

		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			
			glDeleteProgram(program);
			for (const auto id : glShaderIDs)
				glDeleteShader(id);

			ARC_CORE_ERROR("{0}", infoLog.data());
			ARC_CORE_ASSERT(false, "Shader link failure!")
			return;
		}

		for (const auto id : glShaderIDs)
			glDetachShader(program, id);

		m_RendererID = program;

		// Get material properties from the shader
		m_MaterialProperties.clear();
		int maxLength;
		glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);

		int uniformCount;
		glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);

		size_t offset = 0;
		for (GLuint i = 0; i < static_cast<GLuint>(uniformCount); i++)
		{
			char name[128];
			int size;
			GLenum type;
			glGetActiveUniform(program, i, maxLength, nullptr, &size, &type, &name[0]);

			std::string nameStr(name);
			static constexpr const char* prefix = "u_Material.";
			static constexpr size_t prefixOffset = std::string_view(prefix).size();
			if (nameStr.starts_with(prefix))
			{
				const MaterialPropertyType propertyType = GetMaterialPropertyType(type);
				const size_t sizeInBytes = GetSizeInBytes(propertyType);
				const bool isSlider = nameStr.ends_with("01");
				const size_t sufixSize = isSlider ? 2 : 0;
				const bool isColor = nameStr.find("color") != std::string::npos || nameStr.find("Color") != std::string::npos;
				m_MaterialProperties.emplace(nameStr, MaterialProperty{ propertyType, sizeInBytes, offset, nameStr.substr(prefixOffset, nameStr.size() - prefixOffset - sufixSize), isSlider, isColor });
				offset += sizeInBytes;
			}
		}
	}
}
