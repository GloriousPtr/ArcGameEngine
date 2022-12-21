#include "arcpch.h"
#include "Material.h"

#include <glm/gtc/type_ptr.hpp>

#include "Renderer3D.h"
#include "Shader.h"
#include "Texture.h"

namespace ArcEngine
{
	Ref<Texture2D> Material::s_WhiteTexture;

	Material::Material(const char* shaderPath)
	{
		ARC_PROFILE_SCOPE();

		if (!s_WhiteTexture)
		{
			s_WhiteTexture = Texture2D::Create(1, 1);
			uint32_t whiteTextureData = 0xffffffff;
			s_WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
		}

		// Extract name from filepath
		std::string filepath = shaderPath;
		auto lastSlash = std::string(filepath).find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		std::string name = filepath.substr(lastSlash, count);

		if (!Renderer3D::GetShaderLibrary().Exists(name))
			m_Shader = Renderer3D::GetShaderLibrary().Load(shaderPath);
		else
			m_Shader = Renderer3D::GetShaderLibrary().Get(name);

		Invalidate();
	}

	Material::~Material()
	{
		ARC_PROFILE_SCOPE();

		delete[] m_Buffer;
	}

	void Material::Invalidate()
	{
		ARC_PROFILE_SCOPE();

		delete[] m_Buffer;

		m_BufferSizeInBytes = 0;
		const auto& materialProperties = m_Shader->GetMaterialProperties();

		for (const auto& [name, property] : materialProperties)
			m_BufferSizeInBytes += property.SizeInBytes;

		m_Buffer = new char[m_BufferSizeInBytes];
		memset(m_Buffer, 0, m_BufferSizeInBytes);
		
		uint32_t slot = 0;
		glm::vec4 one = glm::vec4(1.0);
		for (auto& [name, property] : materialProperties)
		{
			if (property.Type == MaterialPropertyType::Sampler2D)
			{
				memcpy(m_Buffer + property.OffsetInBytes, &slot, sizeof(uint32_t));
				m_Textures.emplace(slot, nullptr);
				slot++;
			}
			else if (property.Type == MaterialPropertyType::Float ||
				property.Type == MaterialPropertyType::Float2 ||
				property.Type == MaterialPropertyType::Float3 ||
				property.Type == MaterialPropertyType::Float4)
			{
				if (name.find("albedo") != std::string::npos || name.find("Albedo") != std::string::npos)
					memcpy(m_Buffer + property.OffsetInBytes, glm::value_ptr(one), property.SizeInBytes);
				if (name.find("roughness") != std::string::npos || name.find("Roughness") != std::string::npos)
					memcpy(m_Buffer + property.OffsetInBytes, glm::value_ptr(one), property.SizeInBytes);
			}
		}
	}

	void Material::Bind()
	{
		ARC_PROFILE_SCOPE();

		const auto& materialProperties = m_Shader->GetMaterialProperties();

		m_Shader->Bind();
		for (const auto& [name, property] : materialProperties)
		{
			char* bufferStart = m_Buffer + property.OffsetInBytes;
			uint32_t slot = *(uint32_t*)bufferStart;
			switch (property.Type)
			{
				case MaterialPropertyType::None : break;
				case MaterialPropertyType::Sampler2D :
				{
					m_Shader->SetInt(name, slot);
					if (m_Textures.at(slot))
						m_Textures.at(slot)->Bind(slot);
					else
						s_WhiteTexture->Bind(slot);
					break;
				}
				case MaterialPropertyType::Bool :
				case MaterialPropertyType::Int :
				{
					m_Shader->SetInt(name, *((int32_t*)bufferStart));
					break;
				}
				case MaterialPropertyType::Float :
				{
					m_Shader->SetFloat(name, *((float*)bufferStart));
					break;
				}
				case MaterialPropertyType::Float2 :
				{
					m_Shader->SetFloat2(name, *((glm::vec2*)bufferStart));
					break;
				}
				case MaterialPropertyType::Float3 :
				{
					m_Shader->SetFloat3(name, *((glm::vec3*)bufferStart));
					break;
				}
				case MaterialPropertyType::Float4 :
				{
					m_Shader->SetFloat4(name, *((glm::vec4*)bufferStart));
					break;
				}
				default:
				{
					ARC_CORE_ASSERT("Unknown type material property: {}", name.c_str());
					break;
				}
			}
		}
	}

	void Material::Unbind() const
	{
		ARC_PROFILE_SCOPE();

		m_Shader->Unbind();
	}

	Ref<Shader> Material::GetShader() const
	{
		return m_Shader;
	}

	Ref<Texture2D> Material::GetTexture(uint32_t slot)
	{
		return m_Textures.at(slot);
	}

	void Material::SetTexture(uint32_t slot, Ref<Texture2D> texture)
	{
		m_Textures[slot] = texture;
	}

	Material::MaterialData Material::GetData_Internal(const char* name)
	{
		ARC_PROFILE_SCOPE();

		const auto& materialProperties = m_Shader->GetMaterialProperties();
		const auto& materialProperty = m_Shader->GetMaterialProperties().find(name);
		if (materialProperty != materialProperties.end())
			return m_Buffer + materialProperty->second.OffsetInBytes;

		return nullptr;
	}

	void Material::SetData_Internal(const char* name, const Material::MaterialData data)
	{
		ARC_PROFILE_SCOPE();

		const auto& materialProperties = m_Shader->GetMaterialProperties();
		const auto& property = m_Shader->GetMaterialProperties().find(name);
		if (property != materialProperties.end())
			memcpy(m_Buffer + property->second.OffsetInBytes, data, property->second.SizeInBytes);
	}
}
