#pragma once

#include "Arc/Renderer/PipelineState.h"

#include "Dx12Resources.h"

namespace ArcEngine
{
	using BufferMap = std::unordered_map<std::string, uint32_t, UM_StringTransparentEquality>;

	class Dx12PipelineState : public PipelineState
	{
	public:
		Dx12PipelineState(const Ref<Shader>& shader, const PipelineSpecification& spec);
		~Dx12PipelineState() override;

		[[nodiscard]] bool Bind() const override;
		bool Unbind() const override;

		[[nodiscard]] MaterialPropertyMap& GetMaterialProperties() override { return m_MaterialProperties; }
		[[nodiscard]] uint32_t GetSlot(const std::string_view& name) override { return m_BufferMap.at(name.data()); }

	private:
		void SetDataImpl(const std::string& name, const void* value, uint32_t size, uint32_t offset) override;

	private:
		ID3D12RootSignature*		m_RootSignature = nullptr;
		ID3D12PipelineState*		m_PipelineState = nullptr;
		MaterialPropertyMap			m_MaterialProperties;
		BufferMap					m_BufferMap;
		PipelineSpecification		m_Specification;
	};
}
