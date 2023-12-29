#pragma once

#include "Arc/Renderer/PipelineState.h"

#include "Dx12Resources.h"

namespace ArcEngine
{
	class Dx12PipelineState : public PipelineState
	{
	public:
		Dx12PipelineState(const Ref<Shader>& shader, const PipelineSpecification& spec);
		~Dx12PipelineState() override;

		[[nodiscard]] bool Bind() const override;
		bool Unbind() const override;

		[[nodiscard]] eastl::vector<MaterialProperty>& GetMaterialProperties() override { return m_MaterialProperties; }
		[[nodiscard]] uint32_t GetSlot(const eastl::string_view name) override { return m_BufferMap.at(name.begin()); }

	private:
		void SetDataImpl(const eastl::string_view name, const void* value, uint32_t size, uint32_t offset) override;
		void MakeGraphicsPipeline(const Ref<Shader>& shader);
		void MakeComputePipeline(const Ref<Shader>& shader);

	private:
		ID3D12RootSignature*							m_RootSignature = nullptr;
		ID3D12PipelineState*							m_PipelineState = nullptr;
		eastl::vector<MaterialProperty>					m_MaterialProperties;
		eastl::hash_map<eastl::string, uint32_t>		m_BufferMap;
		PipelineSpecification							m_Specification;
	};
}
