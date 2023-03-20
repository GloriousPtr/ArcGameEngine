#include "arcpch.h"
#include "Material.h"

#include <glm/gtc/type_ptr.hpp>

#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
#include "Arc/Core/AssetManager.h"

namespace ArcEngine
{
	Material::Material(const std::filesystem::path& shaderPath)
	{
		ARC_PROFILE_SCOPE()

		// Extract name from filepath
		const std::string name = shaderPath.filename().string();

		if (!Renderer::GetShaderLibrary().Exists(name))
			m_Shader = Renderer::GetShaderLibrary().Load(shaderPath);
		else
			m_Shader = Renderer::GetShaderLibrary().Get(name);

		Invalidate();
	}

	Material::~Material()
	{
		ARC_PROFILE_SCOPE()

		delete[] m_Buffer;
	}

	void Material::Invalidate()
	{
		ARC_PROFILE_SCOPE()

		delete[] m_Buffer;

		m_BufferSizeInBytes = 0;
		const auto& materialProperties = m_Shader->GetMaterialProperties();

		for (const auto& [_, property] : materialProperties)
			m_BufferSizeInBytes += property.SizeInBytes;

		m_Buffer = new char[m_BufferSizeInBytes];
		memset(m_Buffer, 0, m_BufferSizeInBytes);
		
		uint32_t slot = 0;
		auto one = glm::vec4(1.0);
		for (auto& [name, property] : materialProperties)
		{
			if (property.Type == MaterialPropertyType::Texture2D ||
				property.Type == MaterialPropertyType::Texture2DBindless)
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

	void Material::Bind() const
	{
		ARC_PROFILE_SCOPE()

		const auto& materialProperties = m_Shader->GetMaterialProperties();

		m_Shader->Bind();
		for (const auto& [name, property] : materialProperties)
		{
			char* bufferStart = m_Buffer + property.OffsetInBytes;
			uint32_t slot = *reinterpret_cast<uint32_t*>(bufferStart);
			switch (property.Type)
			{
				case MaterialPropertyType::None : break;
				case MaterialPropertyType::Texture2D :
				{
					m_Shader->SetData(name, static_cast<int>(slot), 0, property.BindingOffset);
					if (m_Textures.at(slot))
						m_Textures.at(slot)->Bind(slot);
					else
						AssetManager::WhiteTexture()->Bind(slot);
					break;
				}
				case MaterialPropertyType::Texture2DBindless :
				{
					m_Shader->SetData(name, (m_Textures.at(slot) ? m_Textures.at(slot) : AssetManager::WhiteTexture())->GetIndex(), 0, property.BindingOffset);
					break;
				}
				case MaterialPropertyType::Bool :
				case MaterialPropertyType::Int :
				{
					m_Shader->SetData(name, *reinterpret_cast<int32_t*>(bufferStart), 0, property.BindingOffset);
					break;
				}
				case MaterialPropertyType::UInt :
				{
					m_Shader->SetData(name, *reinterpret_cast<uint32_t*>(bufferStart), 0, property.BindingOffset);
					break;
				}
				case MaterialPropertyType::Float :
				{
					m_Shader->SetData(name, *reinterpret_cast<float*>(bufferStart), 0, property.BindingOffset);
					break;
				}
				case MaterialPropertyType::Float2 :
				{
					m_Shader->SetData(name, *reinterpret_cast<glm::vec2*>(bufferStart), 0, property.BindingOffset);
					break;
				}
				case MaterialPropertyType::Float3 :
				{
					m_Shader->SetData(name, *reinterpret_cast<glm::vec3*>(bufferStart), 0, property.BindingOffset);
					break;
				}
				case MaterialPropertyType::Float4 :
				{
					m_Shader->SetData(name, *reinterpret_cast<glm::vec4*>(bufferStart), 0, property.BindingOffset);
					break;
				}
			}
		}
	}

	void Material::Unbind() const
	{
		ARC_PROFILE_SCOPE()

		m_Shader->Unbind();
	}

	Ref<Shader> Material::GetShader() const
	{
		return m_Shader;
	}

	Ref<Texture2D> Material::GetTexture(uint32_t slot)
	{
		const auto it = m_Textures.find(slot);

		[[likely]]
		if (it != m_Textures.end())
			return it->second;

		return nullptr;
	}

	void Material::SetTexture(uint32_t slot, const Ref<Texture2D>& texture)
	{
		m_Textures[slot] = texture;
	}

	Material::MaterialData Material::GetData_Internal(const std::string& name) const
	{
		ARC_PROFILE_SCOPE()

		const auto& materialProperties = m_Shader->GetMaterialProperties();
		const auto& materialProperty = m_Shader->GetMaterialProperties().find(name);
		if (materialProperty != materialProperties.end())
			return m_Buffer + materialProperty->second.OffsetInBytes;

		return nullptr;
	}

	void Material::SetData_Internal(const std::string& name, const MaterialData data) const
	{
		ARC_PROFILE_SCOPE()

		const auto& materialProperties = m_Shader->GetMaterialProperties();
		const auto& property = m_Shader->GetMaterialProperties().find(name);
		if (property != materialProperties.end())
			memcpy(m_Buffer + property->second.OffsetInBytes, data, property->second.SizeInBytes);
	}
}
