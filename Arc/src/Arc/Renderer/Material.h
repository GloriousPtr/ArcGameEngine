#pragma once

#include "PipelineState.h"

namespace ArcEngine
{
	class Texture2D;
	class ConstantBuffer;

	struct TextureSlot
	{
		uint32_t Slot;
		uint32_t Index;

		TextureSlot(uint32_t slot, uint32_t index)
			: Slot(slot), Index(index)
		{
		}
	};

	struct MaterialData
	{
		MaterialPropertyType Type;
		uint32_t Index;
		uint32_t BufferIndex;
		uint32_t SizeInBytes;


		MaterialData(MaterialPropertyType type, uint32_t index, uint32_t bufferIndex, uint32_t sizeInBytes)
			: Type(type), Index(index), BufferIndex(bufferIndex), SizeInBytes(sizeInBytes)
		{
		}
	};

	class Material
	{
	public:
		explicit Material(const std::filesystem::path& shaderPath = "assets/shaders/PBR.hlsl");
		virtual ~Material();

		Material(const Material& other) = default;
		Material(Material&& other) = default;

		void Invalidate();
		void Bind() const;
		void Unbind() const;

		[[nodiscard]] Ref<Texture2D> GetTexture(const std::string_view& name);
		[[nodiscard]] std::vector<MaterialProperty>& GetProperties() const { return m_Pipeline->GetMaterialProperties(); }

		void SetTexture(const std::string_view& name, const Ref<Texture2D>& texture);

		template<typename T>
		[[nodiscard]] T GetData(const std::string_view& name)
		{
			void* value = GetData_Internal(name);
			return (value ? *static_cast<T*>(value) : T());
		}

		template<typename T>
		void SetData(const std::string_view& name, const T& data)
		{
			SetData_Internal(name, &data);
		}

	private:
		[[nodiscard]] void* GetData_Internal(const std::string_view& name);
		void SetData_Internal(const std::string_view& name, const void* data);

	private:
		Ref<PipelineState>				m_Pipeline = nullptr;
		str_umap<MaterialData>			m_Indices;
		std::vector<TextureSlot>		m_TextureBuffer;
		std::vector<uint32_t>			m_BindlessTextureBuffer;
		std::vector<float>				m_CBBuffer;
		std::vector<Ref<Texture2D>>		m_Textures;
		Ref<ConstantBuffer>				m_ConstantBuffer;
	};
}
