#include "arcpch.h"
#include "Material.h"

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
		ARC_PROFILE_SCOPE();

		// Extract name from filepath
		const eastl::string name = shaderPath.filename().string().c_str();
		ARC_CORE_ASSERT(Renderer::GetPipelineLibrary().Exists(name));

		m_Pipeline = Renderer::GetPipelineLibrary().Get(name);
		Invalidate();
	}

	Material::~Material()
	{
		ARC_PROFILE_SCOPE();
	}

	void Material::Invalidate()
	{
		ARC_PROFILE_SCOPE();

		m_TextureBuffer.clear();
		m_CBBuffer.clear();
		m_Textures.clear();

		const eastl::vector<MaterialProperty>& materialProperties = m_Pipeline->GetMaterialProperties();
		uint32_t whiteTexId = AssetManager::WhiteTexture()->GetIndex();

		glm::vec4 one(1.0);
		int32_t cbSize = 0;
		for (const MaterialProperty& property : materialProperties)
		{
			if (property.Type == MaterialPropertyType::Texture2D)
			{
				const uint32_t texIndex = static_cast<uint32_t>(m_Textures.size());
				const uint32_t buffIndex = static_cast<uint32_t>(m_TextureBuffer.size());
				m_Indices.emplace(property.Name, MaterialData(property.Type, texIndex, buffIndex, property.SizeInBytes));
				m_TextureBuffer.emplace_back(property.Slot, texIndex);
				m_Textures.push_back(nullptr);
			}
			else if (property.Type == MaterialPropertyType::Texture2DBindless)
			{
				const uint32_t texIndex = static_cast<uint32_t>(m_Textures.size());
				const uint32_t buffIndex = static_cast<uint32_t>(m_BindlessTextureBuffer.size());
				m_Indices.emplace(property.Name, MaterialData(property.Type, texIndex, buffIndex, property.SizeInBytes));
				m_BindlessTextureBuffer.emplace_back(whiteTexId);
				m_Textures.push_back(nullptr);
			}
			else
			{
				cbSize = property.StartOffsetInBytes + property.SizeInBytes;

				const uint32_t buffIndex = property.StartOffsetInBytes / sizeof(float);
				m_Indices.emplace(property.Name, MaterialData(property.Type, 0, buffIndex, property.SizeInBytes));
				m_CBBuffer.resize(cbSize / sizeof(float));
				memset(&(m_CBBuffer[buffIndex]), 0, property.SizeInBytes);

				if (property.Type == MaterialPropertyType::Float ||
					property.Type == MaterialPropertyType::Float2 ||
					property.Type == MaterialPropertyType::Float3 ||
					property.Type == MaterialPropertyType::Float4)
				{
					const bool setDefaultValue =	property.Name.find("albedo") != eastl::string::npos ||
													property.Name.find("Albedo") != eastl::string::npos ||
													property.Name.find("roughness") != eastl::string::npos ||
													property.Name.find("Roughness") != eastl::string::npos;

					if (setDefaultValue)
					{
						memcpy(&(m_CBBuffer[buffIndex]), glm::value_ptr(one), property.SizeInBytes);
					}
					if (property.Name.find("AlphaCutoffThreshold") != eastl::string::npos)
					{
						glm::vec4 epsilon(0.01f);
						memcpy(&(m_CBBuffer[buffIndex]), glm::value_ptr(epsilon), property.SizeInBytes);
					}
				}
			}
		}

		m_Pipeline->RegisterCB(CRC32(MaterialPropertiesSlotName), cbSize);
	}

	void Material::Bind(GraphicsCommandList commandList) const
	{
		ARC_PROFILE_SCOPE();

		for (const TextureSlot& t : m_TextureBuffer)
		{
			if (m_Textures[t.Index])
				m_Textures[t.Index]->Bind(commandList, t.Slot);
			else
				AssetManager::WhiteTexture()->Bind(commandList, t.Slot);
		}

		if (!m_BindlessTextureBuffer.empty())
		{
			m_Pipeline->SetRSData(commandList, CRC32(BindlessTexturesSlotName), m_BindlessTextureBuffer.data(), static_cast<uint32_t>(sizeof(uint32_t) * m_BindlessTextureBuffer.size()));
		}
		if (!m_CBBuffer.empty())
		{
			m_Pipeline->SetCBData(commandList, CRC32(MaterialPropertiesSlotName), m_CBBuffer.data(), static_cast<uint32_t>(sizeof(float) * m_CBBuffer.size()));
		}
	}

	Ref<Texture2D> Material::GetTexture(const eastl::string_view name)
	{
		ARC_PROFILE_SCOPE();

		const eastl::hash_map<eastl::string, MaterialData>::iterator it = m_Indices.find_as(name.begin());
		[[likely]]
		if (it != m_Indices.end())
		{
			return m_Textures[it->second.Index];
		}

		return nullptr;
	}

	void Material::SetTexture(const eastl::string_view name, const Ref<Texture2D>& texture)
	{
		ARC_PROFILE_SCOPE();

		const eastl::hash_map<eastl::string, MaterialData>::iterator it = m_Indices.find_as(name.begin());
		[[likely]]
		if (it != m_Indices.end())
		{
			m_Textures[it->second.Index] = texture;
			if (it->second.Type == MaterialPropertyType::Texture2DBindless)
				m_BindlessTextureBuffer[it->second.BufferIndex] = texture ? texture->GetIndex() : AssetManager::WhiteTexture()->GetIndex();
		}
	}

	void* Material::GetData_Internal(const eastl::string_view name)
	{
		ARC_PROFILE_SCOPE();

		const eastl::hash_map<eastl::string, MaterialData>::iterator it = m_Indices.find_as(name.begin());
		[[likely]]
		if (it != m_Indices.end())
		{
			return &(m_CBBuffer[it->second.BufferIndex]);
		}

		return nullptr;
	}

	void Material::SetData_Internal(const eastl::string_view name, const void* data)
	{
		ARC_PROFILE_SCOPE();

		const eastl::hash_map<eastl::string, MaterialData>::iterator it = m_Indices.find_as(name.begin());
		[[likely]]
		if (it != m_Indices.end())
		{
			memcpy(&(m_CBBuffer[it->second.BufferIndex]), data, it->second.SizeInBytes);
		}
	}
}
