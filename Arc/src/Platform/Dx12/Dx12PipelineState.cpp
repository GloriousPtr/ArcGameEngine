#include "arcpch.h"
#include "Dx12PipelineState.h"

#include <comutil.h>
#include <wrl.h>
#include <dxc/inc/dxcapi.h>
#include <dxc/inc/d3d12shader.h>

#include "DxHelper.h"
#include "d3dx12.h"
#include "Dx12Framebuffer.h"
#include "Dx12Shader.h"

namespace ArcEngine
{
	static DXGI_FORMAT GetFormatFromMaskComponents(BYTE mask, D3D_REGISTER_COMPONENT_TYPE componentType)
	{
		if (mask == 1)
		{
			if (componentType == D3D_REGISTER_COMPONENT_UINT32)			return DXGI_FORMAT_R32_UINT;
			if (componentType == D3D_REGISTER_COMPONENT_SINT32)			return DXGI_FORMAT_R32_SINT;
			if (componentType == D3D_REGISTER_COMPONENT_FLOAT32)		return DXGI_FORMAT_R32_FLOAT;
		}
		else if (mask <= 3)
		{
			if (componentType == D3D_REGISTER_COMPONENT_UINT32)			return DXGI_FORMAT_R32G32_UINT;
			if (componentType == D3D_REGISTER_COMPONENT_SINT32)			return DXGI_FORMAT_R32G32_SINT;
			if (componentType == D3D_REGISTER_COMPONENT_FLOAT32)		return DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (mask <= 7)
		{
			if (componentType == D3D_REGISTER_COMPONENT_UINT32)			return DXGI_FORMAT_R32G32B32_UINT;
			if (componentType == D3D_REGISTER_COMPONENT_SINT32)			return DXGI_FORMAT_R32G32B32_SINT;
			if (componentType == D3D_REGISTER_COMPONENT_FLOAT32)		return DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (mask <= 15)
		{
			if (componentType == D3D_REGISTER_COMPONENT_UINT32)			return DXGI_FORMAT_R32G32B32A32_UINT;
			if (componentType == D3D_REGISTER_COMPONENT_SINT32)			return DXGI_FORMAT_R32G32B32A32_SINT;
			if (componentType == D3D_REGISTER_COMPONENT_FLOAT32)		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	static MaterialPropertyType GetVariableType(const D3D12_SHADER_TYPE_DESC& desc)
	{
		switch (desc.Type)
		{
		case D3D_SVT_TEXTURE2D:			return MaterialPropertyType::Texture2D;
		case D3D_SVT_BOOL:				return MaterialPropertyType::Bool;
		case D3D_SVT_INT:				return MaterialPropertyType::Int;
		case D3D_SVT_UINT:				return MaterialPropertyType::UInt;
		case D3D_SVT_FLOAT:
		{
			if (desc.Columns == 1)		return MaterialPropertyType::Float;
			if (desc.Columns == 2)		return MaterialPropertyType::Float2;
			if (desc.Columns == 3)		return MaterialPropertyType::Float3;
			if (desc.Columns == 4)		return MaterialPropertyType::Float4;
		}
		}

		return MaterialPropertyType::None;
	}

	static void AppendMaterials(const Microsoft::WRL::ComPtr<ID3D12ShaderReflection>& reflection,
		std::vector<D3D12_ROOT_PARAMETER1>& outRootParams,
		std::array<D3D12_DESCRIPTOR_RANGE1, 20>& outDescriptors,
		uint32_t* outDescriptorsEnd,
		std::vector<MaterialProperty>& outMaterialProperties,
		BufferMap& bufferMap)
	{
		ARC_PROFILE_SCOPE()

		D3D12_SHADER_DESC shaderDesc;
		reflection->GetDesc(&shaderDesc);

		outRootParams.reserve(outRootParams.size() + shaderDesc.BoundResources);
		for (uint32_t i = 0; i < shaderDesc.BoundResources; ++i)
		{
			D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{};
			reflection->GetResourceBindingDesc(i, &shaderInputBindDesc);

			auto* cb = reflection->GetConstantBufferByIndex(i);
			D3D12_SHADER_BUFFER_DESC constantBufferDesc{};
			cb->GetDesc(&constantBufferDesc);

			std::string bufferName = shaderInputBindDesc.Name;

			if (bufferMap.contains(bufferName))
				continue;

			bool tex = shaderInputBindDesc.Type == D3D_SIT_TEXTURE;
			bool cbuffer = shaderInputBindDesc.Type == D3D_SIT_CBUFFER;
			bool bindlessTextures = cbuffer && bufferName == "Textures";
			bool materialProperties = bufferName == "MaterialProperties";

			CD3DX12_ROOT_PARAMETER1 rootParameter;
			switch (shaderInputBindDesc.Type)
			{
				case D3D_SIT_CBUFFER:
					{
						if (bufferName == "Transform")
						{
							rootParameter.InitAsConstants(16u, shaderInputBindDesc.BindPoint, shaderInputBindDesc.Space);
						}
						else if (bufferName == "Textures")
						{
							rootParameter.InitAsConstants(32u, shaderInputBindDesc.BindPoint, shaderInputBindDesc.Space);
						}
						else
						{
							rootParameter.InitAsConstantBufferView(shaderInputBindDesc.BindPoint, shaderInputBindDesc.Space);
							rootParameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC;
						}
					}
					break;
				case D3D_SIT_TEXTURE:
					{
						CD3DX12_DESCRIPTOR_RANGE1 range;
						range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, shaderInputBindDesc.BindCount, shaderInputBindDesc.BindPoint, shaderInputBindDesc.Space);
						size_t index = *outDescriptorsEnd;
						outDescriptors[index] = range;
						rootParameter.InitAsDescriptorTable(1, &outDescriptors[index]);
						++(*outDescriptorsEnd);
					}
					break;
				case D3D_SIT_UAV_RWTYPED:
					{
						CD3DX12_DESCRIPTOR_RANGE1 range;
						range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, shaderInputBindDesc.BindCount, shaderInputBindDesc.BindPoint, shaderInputBindDesc.Space);
						size_t index = *outDescriptorsEnd;
						outDescriptors[index] = range;
						rootParameter.InitAsDescriptorTable(1, &outDescriptors[index]);
						++(*outDescriptorsEnd);
					}
					break;
				case D3D_SIT_SAMPLER:
				case D3D_SIT_STRUCTURED:
				case D3D_SIT_TBUFFER:
				case D3D_SIT_UAV_RWSTRUCTURED:
				case D3D_SIT_BYTEADDRESS:
				case D3D_SIT_UAV_RWBYTEADDRESS:
				case D3D_SIT_UAV_APPEND_STRUCTURED:
				case D3D_SIT_UAV_CONSUME_STRUCTURED:
				case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
				case D3D_SIT_RTACCELERATIONSTRUCTURE:
				case D3D_SIT_UAV_FEEDBACKTEXTURE:
				default: continue;
			}
			const int32_t slot = static_cast<int32_t>(outRootParams.size());
			outRootParams.emplace_back(rootParameter);

			if (bindlessTextures || materialProperties)
			{
				for (uint32_t var = 0; var < constantBufferDesc.Variables; ++var)
				{
					auto* variable = cb->GetVariableByIndex(var);

					auto* variableType = variable->GetType();
					D3D12_SHADER_TYPE_DESC variableTypeDesc;
					variableType->GetDesc(&variableTypeDesc);
					MaterialPropertyType type = GetVariableType(variableTypeDesc);
					if (type == MaterialPropertyType::None)
					{
						ARC_CORE_ERROR("Unsupported type in shader cbuffer!");
						continue;
					}

					D3D12_SHADER_VARIABLE_DESC variableDesc;
					variable->GetDesc(&variableDesc);

					std::string variableName = variableDesc.Name;

					if (bindlessTextures && type == MaterialPropertyType::UInt)
						type = MaterialPropertyType::Texture2DBindless;

					MaterialProperty property{};
					property.Type = type;
					property.SizeInBytes = variableDesc.Size;
					property.StartOffsetInBytes = variableDesc.StartOffset;
					property.IsSlider = variableName.ends_with("01");
					property.Name = variableDesc.Name;
					property.DisplayName = variableDesc.Name + (property.IsSlider ? 2 : 0);
					property.IsColor = variableName.find("color") != std::string::npos || variableName.find("Color") != std::string::npos;
					property.Slot = slot;

					outMaterialProperties.push_back(property);
					bufferMap.emplace(variableName, slot);
				}
			}
			else if(tex)
			{
				MaterialProperty property{};
				property.Type = MaterialPropertyType::Texture2D;
				property.SizeInBytes = sizeof(uint32_t);
				property.StartOffsetInBytes = 0;
				property.IsSlider = false;
				property.Name = bufferName;
				property.DisplayName = bufferName;
				property.IsColor = false;
				property.Slot = slot;

				outMaterialProperties.push_back(property);
			}

			bufferMap.emplace(bufferName, slot);
		}
	}

	Dx12PipelineState::Dx12PipelineState(const Ref<Shader>& shader, const PipelineSpecification& spec)
		: m_Specification(spec)
	{
		ARC_PROFILE_SCOPE()

		switch(m_Specification.Type)
		{
			case ShaderType::None:  ARC_CORE_ASSERT(false, "Failed to create pipeline with unknown type") break;
			case ShaderType::Vertex:
			case ShaderType::Pixel: MakeGraphicsPipeline(shader); break;
			case ShaderType::Compute: MakeComputePipeline(shader); break;
			default: ARC_CORE_ASSERT(false, "Failed to create pipeline with unknown type")
		}
	}

	Dx12PipelineState::~Dx12PipelineState()
	{
		ARC_PROFILE_SCOPE()

		if (m_PipelineState)
			m_PipelineState->Release();
		if (m_RootSignature)
			m_RootSignature->Release();
	}

	bool Dx12PipelineState::Bind() const
	{
		ARC_PROFILE_SCOPE()

		if (!m_RootSignature || !m_PipelineState)
			return false;

		auto* commandList = Dx12Context::GetGraphicsCommandList();

		ID3D12DescriptorHeap* descriptorHeap = Dx12Context::GetSrvHeap()->Heap();
		commandList->SetDescriptorHeaps(1, &descriptorHeap);

		if (m_Specification.Type == ShaderType::Vertex || m_Specification.Type == ShaderType::Pixel)
			commandList->SetGraphicsRootSignature(m_RootSignature);
		else if (m_Specification.Type == ShaderType::Compute)
			commandList->SetComputeRootSignature(m_RootSignature);

		commandList->SetPipelineState(m_PipelineState);

		return true;
	}

	bool Dx12PipelineState::Unbind() const
	{
		ARC_PROFILE_SCOPE()

		if (!m_RootSignature || !m_PipelineState)
			return false;

		auto* commandList = Dx12Context::GetGraphicsCommandList();
		commandList->SetGraphicsRootSignature(nullptr);
		commandList->SetPipelineState(nullptr);
		return true;
	}

	void Dx12PipelineState::SetDataImpl(const std::string& name, const void* data, uint32_t size, uint32_t offset)
	{
		ARC_PROFILE_SCOPE()

		const int32_t slot = m_BufferMap.at(name);
		Dx12Context::GetGraphicsCommandList()->SetGraphicsRoot32BitConstants(slot, size / 4, data, offset);
	}

	void Dx12PipelineState::MakeGraphicsPipeline(const Ref<Shader>& shader)
	{
		using Microsoft::WRL::ComPtr;

		const auto& graphicsSpec = m_Specification.GraphicsPipelineSpecs;

		const auto* dxShader = reinterpret_cast<const Dx12Shader*>(shader.get());
		const ComPtr<IDxcBlob> vertexShader = dxShader->m_ShaderBlobs.at(ShaderType::Vertex);
		const ComPtr<IDxcBlob> pixelShader = dxShader->m_ShaderBlobs.at(ShaderType::Pixel);
		const ComPtr<IDxcBlob> vertexReflection = dxShader->m_ReflectionBlobs.at(ShaderType::Vertex);
		const ComPtr<IDxcBlob> pixelReflection = dxShader->m_ReflectionBlobs.at(ShaderType::Pixel);

		ComPtr<IDxcUtils> utils = nullptr;
		HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
		if (FAILED(hr))
		{
			ARC_CORE_ERROR("DxcUtils creation failed!");
			return;
		}

		_bstr_t shaderName = ToWCSTR(shader->GetName().c_str());
		struct Layout
		{
			std::string Name;
			uint32_t Index;
			DXGI_FORMAT Format;
		};
		std::vector<Layout> inputLayout;
		std::vector<Layout> outputLayout;
		std::vector<D3D12_ROOT_PARAMETER1> rootParams;
		std::array<D3D12_DESCRIPTOR_RANGE1, 20> rootDescriptors;
		uint32_t rootDescriptorsEnd = 0;

		if (vertexReflection)
		{
			// Create reflection interface.
			DxcBuffer reflectionData{};
			reflectionData.Encoding = DXC_CP_ACP;
			reflectionData.Ptr = vertexReflection->GetBufferPointer();
			reflectionData.Size = vertexReflection->GetBufferSize();

			ComPtr<ID3D12ShaderReflection> reflect;
			utils->CreateReflection(&reflectionData, IID_PPV_ARGS(&reflect));

			// Use reflection interface here.
			D3D12_SHADER_DESC shaderDesc;
			reflect->GetDesc(&shaderDesc);

			// Create InputLayout from reflection
			for (uint32_t i = 0; i < shaderDesc.InputParameters; ++i)
			{
				D3D12_SIGNATURE_PARAMETER_DESC desc;
				reflect->GetInputParameterDesc(i, &desc);

				DXGI_FORMAT format = GetFormatFromMaskComponents(desc.Mask, desc.ComponentType);
				if (format == DXGI_FORMAT_UNKNOWN)
				{
					ARC_CORE_ERROR("Unknown format for SemanticName: {}, SemanticIndex: {}", desc.SemanticName, desc.SemanticIndex);
					return;
				}
				inputLayout.emplace_back(desc.SemanticName, desc.SemanticIndex, format);
			}

			// Get root parameters from shader reflection data.
			AppendMaterials(reflect, rootParams, rootDescriptors, &rootDescriptorsEnd, m_MaterialProperties, m_BufferMap);
		}

		if (pixelReflection)
		{
			// Create reflection interface.
			DxcBuffer reflectionData{};
			reflectionData.Encoding = DXC_CP_ACP;
			reflectionData.Ptr = pixelReflection->GetBufferPointer();
			reflectionData.Size = pixelReflection->GetBufferSize();

			ComPtr<ID3D12ShaderReflection> reflect;
			utils->CreateReflection(&reflectionData, IID_PPV_ARGS(&reflect));

			// Use reflection interface here.
			D3D12_SHADER_DESC shaderDesc;
			reflect->GetDesc(&shaderDesc);

			if (graphicsSpec.OutputFormats.empty())
			{
				ARC_CORE_WARN("Generating output layout from reflection as none was provided to the pipeline: {}", shader->GetName());
				for (uint32_t i = 0; i < shaderDesc.OutputParameters; ++i)
				{
					D3D12_SIGNATURE_PARAMETER_DESC desc;
					reflect->GetOutputParameterDesc(i, &desc);

					DXGI_FORMAT format = GetFormatFromMaskComponents(desc.Mask, desc.ComponentType);
					if (format == DXGI_FORMAT_UNKNOWN)
					{
						ARC_CORE_ERROR("Unknown format for SemanticName: {}, SemanticIndex: {}", desc.SemanticName, desc.SemanticIndex);
						return;
					}
					outputLayout.emplace_back(desc.SemanticName, desc.SemanticIndex, format);
				}
			}

			AppendMaterials(reflect, rootParams, rootDescriptors, &rootDescriptorsEnd, m_MaterialProperties, m_BufferMap);
		}

		std::vector<D3D12_INPUT_ELEMENT_DESC> psoInputLayout;
		psoInputLayout.reserve(inputLayout.size());
		for (auto& i : inputLayout)
		{
			constexpr auto classification = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			psoInputLayout.emplace_back(i.Name.c_str(), i.Index, i.Format, 0, D3D12_APPEND_ALIGNED_ELEMENT, classification, 0);
		}


		constexpr uint32_t numSamplers = 1;
		CD3DX12_STATIC_SAMPLER_DESC samplers[1];
		samplers[0].Init(0, D3D12_FILTER_ANISOTROPIC);

		// Create root signature.
		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc
		{
			.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
			.Desc_1_1
			{
				.NumParameters = static_cast<uint32_t>(rootParams.size()),
				.pParameters = rootParams.data(),
				.NumStaticSamplers = numSamplers,
				.pStaticSamplers = samplers,
				.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
			}
		};

		ComPtr<ID3DBlob> rootBlob;
		ComPtr<ID3DBlob> errorBlob;
		hr = D3D12SerializeVersionedRootSignature(&rootSigDesc, &rootBlob, &errorBlob);
		if (FAILED(hr))
		{
			if (errorBlob)
				ARC_CORE_ERROR("Failed to serialize Root Signature. Error: {}", static_cast<const char*>(errorBlob->GetBufferPointer()));

			return;
		}

		hr = Dx12Context::GetDevice()->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
		if (FAILED(hr))
		{
			ARC_CORE_ERROR("Failed to create Root Signature. Shader: {}", shader->GetName());
			return;
		}
		m_RootSignature->SetName(shaderName);












		// PSO /////////////////////////////////////////////////////////////////////
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = m_RootSignature;
		psoDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
		psoDesc.VS.BytecodeLength = vertexShader->GetBufferSize();
		psoDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
		psoDesc.PS.BytecodeLength = pixelShader->GetBufferSize();

		// BlendState
		D3D12_RENDER_TARGET_BLEND_DESC blendDesc;
		blendDesc.BlendEnable = TRUE;
		blendDesc.LogicOpEnable = FALSE;
		blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
		blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
		psoDesc.BlendState.IndependentBlendEnable = FALSE;
		for (auto& renderTargetDesc : psoDesc.BlendState.RenderTarget)
			renderTargetDesc = blendDesc;

		// RasterizerState
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		switch (graphicsSpec.CullMode)
		{
			case CullModeType::None:		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; break;
			case CullModeType::Back:		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK; break;
			case CullModeType::Front:		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT; break;
		}
		switch (graphicsSpec.FillMode)
		{
			case FillModeType::Solid:		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; break;
			case FillModeType::Wireframe:	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME; break;
		}

		// DepthStencilState
		psoDesc.DepthStencilState.StencilEnable = FALSE;

		// InputLayout
		psoDesc.InputLayout.NumElements = static_cast<uint32_t>(psoInputLayout.size());
		psoDesc.InputLayout.pInputElementDescs = psoInputLayout.data();

		psoDesc.SampleMask = 0xFFFFFFFF;
		switch (graphicsSpec.Primitive)
		{
			case PrimitiveType::Triangle:	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; break;
			case PrimitiveType::Line:		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; break;
			case PrimitiveType::Point:		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT; break;
		}

		if (graphicsSpec.EnableDepth && Framebuffer::IsDepthFormat(graphicsSpec.DepthFormat))
		{
			psoDesc.DSVFormat = Dx12Framebuffer::GetDxgiFormat(graphicsSpec.DepthFormat);
			psoDesc.DepthStencilState.DepthEnable = true;
		}
		else
		{
			psoDesc.DepthStencilState.DepthEnable = false;
		}
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

		switch (graphicsSpec.DepthFunc)
		{
			case DepthFuncType::None:			psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_NONE; break;
			case DepthFuncType::Never:			psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_NEVER; break;
			case DepthFuncType::Less:			psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS; break;
			case DepthFuncType::Equal:			psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL; break;
			case DepthFuncType::LessEqual:		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; break;
			case DepthFuncType::Greater:		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER; break;
			case DepthFuncType::NotEqual:		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL; break;
			case DepthFuncType::GreaterEqual:	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL; break;
			case DepthFuncType::Always:			psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS; break;
		}

		if (!outputLayout.empty())
		{
			psoDesc.NumRenderTargets = static_cast<uint32_t>(outputLayout.size());
			for (size_t i = 0; i < outputLayout.size(); ++i)
				psoDesc.RTVFormats[i] = outputLayout[i].Format;
		}
		else
		{
			const uint32_t numRenderTargets = static_cast<uint32_t>(graphicsSpec.OutputFormats.size());
			psoDesc.NumRenderTargets = numRenderTargets;
			for (size_t i = 0; i < numRenderTargets; ++i)
				psoDesc.RTVFormats[i] = Dx12Framebuffer::GetDxgiFormat(graphicsSpec.OutputFormats[i]);
		}
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.NodeMask = 0;

		hr = Dx12Context::GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState));
		if (FAILED(hr))
		{
			ARC_CORE_ERROR("Failed to create Pipeline State. Shader: {}", shader->GetName());
		}

		m_PipelineState->SetName(shaderName);
	}

	void Dx12PipelineState::MakeComputePipeline(const Ref<Shader>& shader)
	{
		using Microsoft::WRL::ComPtr;

		const auto* dxShader = reinterpret_cast<const Dx12Shader*>(shader.get());
		const ComPtr<IDxcBlob> computeShader = dxShader->m_ShaderBlobs.at(ShaderType::Compute);
		const ComPtr<IDxcBlob> computeReflection = dxShader->m_ReflectionBlobs.at(ShaderType::Compute);

		ComPtr<IDxcUtils> utils = nullptr;
		HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
		if (FAILED(hr))
		{
			ARC_CORE_ERROR("DxcUtils creation failed!");
			return;
		}

		_bstr_t shaderName = ToWCSTR(shader->GetName().c_str());
		std::vector<D3D12_ROOT_PARAMETER1> rootParams;
		std::array<D3D12_DESCRIPTOR_RANGE1, 20> rootDescriptors;
		uint32_t rootDescriptorsEnd = 0;

		if (computeReflection)
		{
			// Create reflection interface.
			DxcBuffer reflectionData{};
			reflectionData.Encoding = DXC_CP_ACP;
			reflectionData.Ptr = computeReflection->GetBufferPointer();
			reflectionData.Size = computeReflection->GetBufferSize();

			ComPtr<ID3D12ShaderReflection> reflect;
			utils->CreateReflection(&reflectionData, IID_PPV_ARGS(&reflect));

			// Use reflection interface here.
			D3D12_SHADER_DESC shaderDesc;
			reflect->GetDesc(&shaderDesc);

			// Get root parameters from shader reflection data.
			AppendMaterials(reflect, rootParams, rootDescriptors, &rootDescriptorsEnd, m_MaterialProperties, m_BufferMap);
		}

		constexpr uint32_t numSamplers = 1;
		CD3DX12_STATIC_SAMPLER_DESC samplers[1];
		samplers[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

		// Create root signature.
		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc
		{
			.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
			.Desc_1_1
			{
				.NumParameters = static_cast<uint32_t>(rootParams.size()),
				.pParameters = rootParams.data(),
				.NumStaticSamplers = numSamplers,
				.pStaticSamplers = samplers,
				.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE
			}
		};

		ComPtr<ID3DBlob> rootBlob;
		ComPtr<ID3DBlob> errorBlob;
		hr = D3D12SerializeVersionedRootSignature(&rootSigDesc, &rootBlob, &errorBlob);
		if (FAILED(hr))
		{
			if (errorBlob)
				ARC_CORE_ERROR("Failed to serialize Root Signature. Error: {}", static_cast<const char*>(errorBlob->GetBufferPointer()));

			return;
		}

		auto* device = Dx12Context::GetDevice();

		hr = device->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
		if (FAILED(hr))
		{
			ARC_CORE_ERROR("Failed to create Root Signature. Shader: {}", shader->GetName());
			return;
		}
		m_RootSignature->SetName(shaderName);

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = m_RootSignature;
		psoDesc.CS.pShaderBytecode = computeShader->GetBufferPointer();
		psoDesc.CS.BytecodeLength = computeShader->GetBufferSize();

		hr = device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState));
		if (FAILED(hr))
		{
			ARC_CORE_ERROR("Failed to create Pipeline State. Shader: {}", shader->GetName());
		}

		m_PipelineState->SetName(shaderName);
	}
}
