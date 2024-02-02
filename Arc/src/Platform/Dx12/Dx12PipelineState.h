#pragma once

#include "Arc/Renderer/PipelineState.h"

#include "Dx12Resources.h"

namespace D3D12MA
{
	class Allocation;
}

namespace ArcEngine
{
	class Dx12PipelineState : public PipelineState
	{
	public:
		Dx12PipelineState(const Ref<Shader>& shader, const PipelineSpecification& spec);
		~Dx12PipelineState() override;

		[[nodiscard]] bool Bind(GraphicsCommandList commandList) const override;
		bool Unbind(GraphicsCommandList commandList) const override;

		[[nodiscard]] eastl::vector<MaterialProperty>& GetMaterialProperties() override { return m_MaterialProperties; }
		[[nodiscard]] uint32_t GetSlot(const eastl::string_view name) override { return m_BufferMap.at(name.begin()); }

		virtual void RegisterCB(eastl::string_view name, uint32_t size) override;
		virtual void RegisterSB(eastl::string_view name, uint32_t stride, uint32_t count) override;

		virtual void BindCB(GraphicsCommandList commandList, uint32_t crc) override;
		virtual void BindSB(GraphicsCommandList commandList, uint32_t crc) override;

		virtual void SetRSData(GraphicsCommandList commandList, eastl::string_view name, const void* data, uint32_t size, uint32_t offset = 0) override;
		virtual void SetCBData(GraphicsCommandList commandList, uint32_t crc, const void* data, uint32_t size, uint32_t offset = 0) override;
		virtual void SetSBData(GraphicsCommandList commandList, uint32_t crc, const void* data, uint32_t size, uint32_t index) override;

	private:
		void MakeGraphicsPipeline(const Ref<Shader>& shader);
		void MakeComputePipeline(const Ref<Shader>& shader);

	private:
		struct ConstantBuffer
		{
			DescriptorHandle		Handle[Dx12Context::FrameCount]{};
			D3D12MA::Allocation*	Allocation[Dx12Context::FrameCount]{};
			uint32_t				RegisterIndex = 0;
			uint32_t				AlignedSize = 0;
		};

		struct StructuredBuffer
		{
			DescriptorHandle		Handle[Dx12Context::FrameCount]{};
			D3D12MA::Allocation*	Allocation[Dx12Context::FrameCount]{};
			uint32_t				RegisterIndex = 0;
			uint32_t				Stride = 0;
			uint32_t				Count = 0;
		};

		ID3D12RootSignature*									m_RootSignature = nullptr;
		ID3D12PipelineState*									m_PipelineState = nullptr;
		eastl::vector<MaterialProperty>							m_MaterialProperties;
		eastl::hash_map<eastl::string, uint32_t>				m_BufferMap;
		eastl::vector_map<uint32_t, ConstantBuffer>				m_ConstantBufferMap;
		eastl::vector_map<uint32_t, StructuredBuffer>			m_StructuredBufferMap;
		PipelineSpecification									m_Specification;
	};
}
