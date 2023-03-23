#include "arcpch.h"
#include "Material.h"

#include <ranges>
#include <glm/gtc/type_ptr.hpp>

#include "Buffer.h"
#include "Renderer.h"
#include "Shader.h"
#include "PipelineState.h"
#include "Texture.h"
#include "Arc/Core/AssetManager.h"

namespace ArcEngine
{
	Material::Material(const std::filesystem::path& shaderPath)
	{
		ARC_PROFILE_SCOPE()

		// Extract name from filepath
		const std::string name = shaderPath.filename().string();
		ARC_CORE_ASSERT(Renderer::GetPipelineLibrary().Exists(name))

		m_Pipeline = Renderer::GetPipelineLibrary().Get(name);
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
		const auto& materialProperties = m_Pipeline->GetMaterialProperties();

		for (const auto& property : materialProperties | std::views::values)
			m_BufferSizeInBytes += property.SizeInBytes;

		m_Buffer = new char[m_BufferSizeInBytes];
		memset(m_Buffer, 0, m_BufferSizeInBytes);
		
		uint32_t slot = 0;
		auto one = glm::vec4(1.0, 0.0f, 1.0f, 1.0f);
		for (auto& [name, property] : materialProperties)
		{
			int slot = property.Slot;
			if (property.Type == MaterialPropertyType::Texture2D ||
				property.Type == MaterialPropertyType::Texture2DBindless)
			{
				memcpy(m_Buffer + property.OffsetInBytes, &slot, sizeof(uint32_t));
				m_Textures.emplace(slot, nullptr);
			}
			else
			{
				m_ConstantBuffers.emplace(slot, ConstantBuffer::Create(static_cast<uint32_t>(property.SizeInBytes), 1, property.Slot));
			}

			if (property.Type == MaterialPropertyType::Float ||
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

		const auto& materialProperties = m_Pipeline->GetMaterialProperties();

		[[likely]]
		if (m_Pipeline->Bind())
		{
			for (const auto& [name, property] : materialProperties)
			{
				const uint32_t slot = property.Slot;

				switch (property.Type)
				{
					case MaterialPropertyType::None: break;
					case MaterialPropertyType::Texture2D:
					{
						if (m_Textures.at(slot))
							m_Textures.at(slot)->Bind(property.Slot);
						else
							AssetManager::WhiteTexture()->Bind(property.Slot);
						break;
					}
					case MaterialPropertyType::Texture2DBindless:
					{
						const uint32_t index = (m_Textures.at(slot) ? m_Textures.at(slot) : AssetManager::WhiteTexture())->GetIndex();
						m_Pipeline->SetData(name, &index, sizeof(uint32_t), property.BindingOffset);
						break;
					}
					case MaterialPropertyType::Bool:
					case MaterialPropertyType::Int:
					case MaterialPropertyType::UInt:
					case MaterialPropertyType::Float:
					case MaterialPropertyType::Float2:
					case MaterialPropertyType::Float3:
					case MaterialPropertyType::Float4:
					{
						const Ref<ConstantBuffer> constantBuffer = m_ConstantBuffers.at(slot);
						constantBuffer->Bind(0);
						constantBuffer->SetData(m_Buffer + property.OffsetInBytes, static_cast<uint32_t>(property.SizeInBytes), 0);
						break;
					}
				}
			}
		}
	}

	void Material::Unbind() const
	{
		ARC_PROFILE_SCOPE()

		m_Pipeline->Unbind();
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

		const auto& materialProperties = m_Pipeline->GetMaterialProperties();
		const auto& materialProperty = m_Pipeline->GetMaterialProperties().find(name);
		if (materialProperty != materialProperties.end())
			return m_Buffer + materialProperty->second.OffsetInBytes;

		return nullptr;
	}

	void Material::SetData_Internal(const std::string& name, const MaterialData data) const
	{
		ARC_PROFILE_SCOPE()

		const auto& materialProperties = m_Pipeline->GetMaterialProperties();
		const auto& property = m_Pipeline->GetMaterialProperties().find(name);
		if (property != materialProperties.end())
			memcpy(m_Buffer + property->second.OffsetInBytes, data, property->second.SizeInBytes);
	}
}
