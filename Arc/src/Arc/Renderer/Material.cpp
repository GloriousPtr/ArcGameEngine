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
	}

	void Material::Invalidate()
	{
		ARC_PROFILE_SCOPE()

		m_TextureBuffer.clear();
		m_CBBuffer.clear();
		m_Textures.clear();
		m_ConstantBuffer.reset();

		const auto& materialProperties = m_Pipeline->GetMaterialProperties();
		uint32_t whiteTexId = AssetManager::WhiteTexture()->GetIndex();

		glm::vec4 one(1.0);
		size_t cbSize = 0;
		uint32_t cbSlot = 0;
		for (auto& property : materialProperties)
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
				cbSlot = property.Slot;

				const uint32_t buffIndex = property.StartOffsetInBytes / sizeof(float);
				m_Indices.emplace(property.Name, MaterialData(property.Type, 0, buffIndex, property.SizeInBytes));
				m_CBBuffer.resize(cbSize / sizeof(float));
				memset(&(m_CBBuffer[buffIndex]), 0, property.SizeInBytes);

				if (property.Type == MaterialPropertyType::Float ||
					property.Type == MaterialPropertyType::Float2 ||
					property.Type == MaterialPropertyType::Float3 ||
					property.Type == MaterialPropertyType::Float4)
				{
					const bool setDefaultValue =	property.Name.find("albedo") != std::string::npos ||
													property.Name.find("Albedo") != std::string::npos ||
													property.Name.find("roughness") != std::string::npos ||
													property.Name.find("Roughness") != std::string::npos;

					if (setDefaultValue)
					{
						memcpy(&(m_CBBuffer[buffIndex]), glm::value_ptr(one), property.SizeInBytes);
					}
					if (property.Name.find("AlphaCutoffThreshold") != std::string::npos)
					{
						glm::vec4 epsilon(0.01f);
						memcpy(&(m_CBBuffer[buffIndex]), glm::value_ptr(epsilon), property.SizeInBytes);
					}
				}
			}
		}

		m_ConstantBuffer = ConstantBuffer::Create(static_cast<uint32_t>(cbSize), 1, cbSlot);
	}

	void Material::Bind() const
	{
		ARC_PROFILE_SCOPE()

		[[likely]]
		if (m_Pipeline->Bind())
		{
			for (auto& t : m_TextureBuffer)
			{
				if (m_Textures[t.Index])
					m_Textures[t.Index]->Bind(t.Slot);
				else
					AssetManager::WhiteTexture()->Bind(t.Slot);
			}

			if (!m_BindlessTextureBuffer.empty())
			{
				m_Pipeline->SetData("Textures", m_BindlessTextureBuffer.data(), static_cast<uint32_t>(sizeof(uint32_t) * m_BindlessTextureBuffer.size()), 0);
			}
			if (!m_CBBuffer.empty())
			{
				m_ConstantBuffer->Bind(0);
				m_ConstantBuffer->SetData(m_CBBuffer.data(), static_cast<uint32_t>(sizeof(float) * m_CBBuffer.size()), 0);
			}
		}
	}

	void Material::Unbind() const
	{
		ARC_PROFILE_SCOPE()

		m_Pipeline->Unbind();
	}

	Ref<Texture2D> Material::GetTexture(const std::string_view& name)
	{
		ARC_PROFILE_SCOPE()

		const auto it = m_Indices.find(name.data());
		[[likely]]
		if (it != m_Indices.end())
		{
			return m_Textures[it->second.Index];
		}

		return nullptr;
	}

	void Material::SetTexture(const std::string_view& name, const Ref<Texture2D>& texture)
	{
		ARC_PROFILE_SCOPE()

		const auto it = m_Indices.find(name.data());
		[[likely]]
		if (it != m_Indices.end())
		{
			m_Textures[it->second.Index] = texture;
			if (it->second.Type == MaterialPropertyType::Texture2DBindless)
				m_BindlessTextureBuffer[it->second.BufferIndex] = texture ? texture->GetIndex() : AssetManager::WhiteTexture()->GetIndex();
		}
	}

	void* Material::GetData_Internal(const std::string_view& name)
	{
		ARC_PROFILE_SCOPE()

		const auto it = m_Indices.find(name.data());
		[[likely]]
		if (it != m_Indices.end())
		{
			return &(m_CBBuffer[it->second.BufferIndex]);
		}

		return nullptr;
	}

	void Material::SetData_Internal(const std::string_view& name, const void* data)
	{
		ARC_PROFILE_SCOPE()

		const auto it = m_Indices.find(name.data());
		[[likely]]
		if (it != m_Indices.end())
		{
			memcpy(&(m_CBBuffer[it->second.BufferIndex]), data, it->second.SizeInBytes);
		}
	}
}
