#include <arcpch.h>
#include "Dx12Shader.h"

#include "d3dx12.h"

#include <comutil.h>
#include <dxc/inc/dxcapi.h>
#include <dxc/inc/d3d12shader.h>
#include <wrl.h>

#include <glm/gtc/type_ptr.hpp>

namespace ArcEngine
{
	Dx12Shader::Dx12Shader(const std::filesystem::path& filepath)
	{
		ARC_PROFILE_SCOPE()

		m_Name = filepath.filename().string();
		Compile(filepath);
	}

	Dx12Shader::~Dx12Shader()
	{
		ARC_PROFILE_SCOPE()

		if (m_PipelineState)
			m_PipelineState->Release();
		if(m_RootSignature)
			m_RootSignature->Release();
	}

	void Dx12Shader::Recompile(const std::filesystem::path& path)
	{
		if (m_PipelineState)
			m_PipelineState->Release();
		if (m_RootSignature)
			m_RootSignature->Release();

		Compile(path);
	}

	void Dx12Shader::Bind() const
	{
		ARC_PROFILE_SCOPE()

		if (!m_RootSignature || !m_PipelineState)
			return;

		auto* commandList = Dx12Context::GetGraphicsCommandList();
		commandList->SetGraphicsRootSignature(m_RootSignature);
		commandList->SetPipelineState(m_PipelineState);
	}

	void Dx12Shader::Unbind() const
	{
	}

	void Dx12Shader::SetInt(const std::string& name, int value, uint32_t offset)
	{
		SetData(m_MaterialProperties.at(name).Slot, 1, &value, offset);
	}

	void Dx12Shader::SetUInt(const std::string& name, unsigned int value, uint32_t offset)
	{
		SetData(m_MaterialProperties.at(name).Slot, 1, &value, offset);
	}

	void Dx12Shader::SetIntArray(const std::string& name, const int* values, uint32_t count, uint32_t offset)
	{
		SetData(m_MaterialProperties.at(name).Slot, count, values, offset);
	}

	void Dx12Shader::SetFloat(const std::string& name, float value, uint32_t offset)
	{
		SetData(m_MaterialProperties.at(name).Slot, 1, &value, offset);
	}

	void Dx12Shader::SetFloat2(const std::string& name, const glm::vec2& value, uint32_t offset)
	{
		SetData(m_MaterialProperties.at(name).Slot, 2, glm::value_ptr(value), offset);
	}

	void Dx12Shader::SetFloat3(const std::string& name, const glm::vec3& value, uint32_t offset)
	{
		SetData(m_MaterialProperties.at(name).Slot, 3, glm::value_ptr(value), offset);
	}

	void Dx12Shader::SetFloat4(const std::string& name, const glm::vec4& value, uint32_t offset)
	{
		SetData(m_MaterialProperties.at(name).Slot, 4, glm::value_ptr(value), offset);
	}

	void Dx12Shader::SetMat3(const std::string& name, const glm::mat3& value, uint32_t offset)
	{
		SetData(m_MaterialProperties.at(name).Slot, 3 * 3, glm::value_ptr(value), offset);
	}

	void Dx12Shader::SetMat4(const std::string& name, const glm::mat4& value, uint32_t offset)
	{
		SetData(m_MaterialProperties.at(name).Slot, 4 * 4, glm::value_ptr(value), offset);
	}

	void Dx12Shader::SetData(uint32_t slot, uint32_t num32BitValues, const void* data, uint32_t offset)
	{
		Dx12Context::GetGraphicsCommandList()->SetGraphicsRoot32BitConstants(slot, num32BitValues, data, offset);
	}

	const std::string& Dx12Shader::GetName() const
	{
		return m_Name;
	}



	enum class ShaderModel
	{
		None = 0,
		Vertex,
		Fragment,

		Pixel = Fragment
	};

	inline static std::map<ShaderModel, const wchar_t*> s_TargetMap
	{
		{ ShaderModel::Vertex,		L"vs_6_6" },
		{ ShaderModel::Fragment,	L"ps_6_6" },
	};

	inline static std::map<ShaderModel, const wchar_t*> s_EntryPointMap
	{
		{ ShaderModel::Vertex,		L"VS_Main" },
		{ ShaderModel::Fragment,	L"PS_Main" },
	};

	static Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		const std::filesystem::path& filepath,
		const Microsoft::WRL::ComPtr<IDxcBlobEncoding>& encodedBlob,
		const Microsoft::WRL::ComPtr<IDxcIncludeHandler>& includeHandler,
		ShaderModel shaderModel,
		const std::vector<const wchar_t*>& arguments,
		const std::vector<const wchar_t*>& defines,
		Microsoft::WRL::ComPtr<IDxcBlob>* reflection)
	{
		using namespace Microsoft::WRL;

		if (!s_TargetMap.contains(shaderModel) || !s_EntryPointMap.contains(shaderModel))
		{
			ARC_CORE_ERROR("Shader model not found for: {}", filepath);
			return nullptr;
		}

		ComPtr<IDxcCompiler3> compiler;
		HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
		if (FAILED(hr))
		{
			ARC_CORE_ERROR("DxcCompiler creation failed!");
			return nullptr;
		}

		// Prepare args
		std::vector args
		{
			filepath.filename().c_str(),
			L"-E", s_EntryPointMap.at(shaderModel),
			L"-T", s_TargetMap.at(shaderModel),
		};
		args.reserve(args.size() + arguments.size() + defines.size());
		
		for (const auto& arg : arguments)
			args.push_back(arg);

		for (const auto& define : defines)
		{
			args.push_back(L"-D");
			args.push_back(define);
		}

		DxcBuffer shaderBuffer{};
		shaderBuffer.Encoding = DXC_CP_ACP;
		shaderBuffer.Ptr = encodedBlob->GetBufferPointer();
		shaderBuffer.Size = encodedBlob->GetBufferSize();
		ComPtr<IDxcResult> compileResult;
		hr = compiler->Compile(&shaderBuffer, args.data(), static_cast<uint32_t>(args.size()), includeHandler.Get(), IID_PPV_ARGS(&compileResult));
		if (SUCCEEDED(hr) && compileResult)
			compileResult->GetStatus(&hr);

		if (FAILED(hr))
		{
			if (compileResult)
			{
				ComPtr<IDxcBlobUtf8> errors;
				compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
				if (errors && errors->GetStringLength() > 0)
					ARC_CORE_ERROR("Shader compilation error: {}", errors->GetStringPointer());
				else
					ARC_CORE_ERROR("Shader compilation error with no information!");
			}

			return nullptr;
		}

		if (reflection)
			compileResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(reflection->GetAddressOf()), nullptr);

		ComPtr<IDxcBlob> shader;
		compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr);
		return shader;
	}

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
		std::vector<D3D12_ROOT_PARAMETER>& outRootParams,
		std::vector<D3D12_DESCRIPTOR_RANGE>& outDescriptors,
		MaterialPropertyMap& outMaterialProperties)
	{
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

			bool supported = shaderInputBindDesc.Type != D3D_SIT_SAMPLER;
			bool tex = shaderInputBindDesc.Type == D3D_SIT_TEXTURE;
			bool cbuffer = shaderInputBindDesc.Type == D3D_SIT_CBUFFER;
			bool rootConstants = cbuffer && shaderInputBindDesc.BindPoint == 0;

			bool bindlessTextures = std::string("Textures") == shaderInputBindDesc.Name;
			bool materialProperties = std::string("MaterialProperties") == shaderInputBindDesc.Name;
			if (supported)
			{
				CD3DX12_ROOT_PARAMETER rootParameter;
				if (rootConstants)
				{
					rootParameter.InitAsConstants(glm::min(64u, constantBufferDesc.Size / 4), shaderInputBindDesc.BindPoint, shaderInputBindDesc.Space);
				}
				else if (tex)
				{
					CD3DX12_DESCRIPTOR_RANGE range;
					range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, shaderInputBindDesc.BindCount, shaderInputBindDesc.BindPoint, shaderInputBindDesc.Space);
					size_t index = outDescriptors.size();
					outDescriptors.push_back(range);
					rootParameter.InitAsDescriptorTable(1, &outDescriptors[index]);
				}
				else if (cbuffer)
				{
					rootParameter.InitAsConstantBufferView(shaderInputBindDesc.BindPoint, shaderInputBindDesc.Space);
				}
				outRootParams.emplace_back(rootParameter);

				if (bindlessTextures || materialProperties)
				{
					uint32_t offset = 0;
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
						property.OffsetInBytes = offset;
						property.BindingOffset = var;
						property.IsSlider = variableName.ends_with("01");
						property.DisplayName = variableDesc.Name + (property.IsSlider ? 2 : 0);
						property.IsColor = variableName.find("color") != std::string::npos || variableName.find("Color") != std::string::npos;
						property.Slot = static_cast<int32_t>(outRootParams.size()) - 1;

						outMaterialProperties.emplace(variableDesc.Name, property);

						offset += variableDesc.Size;
					}
				}
			}
		}
	}

	void Dx12Shader::Compile(const std::filesystem::path& filepath)
	{
		using namespace Microsoft::WRL;
		
		ComPtr<IDxcUtils> utils = nullptr;
		HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
		if (FAILED(hr))
		{
			ARC_CORE_ERROR("DxcUtils creation failed!");
			return;
		}

		ComPtr<IDxcBlobEncoding> shaderSource;
		hr = utils->LoadFile(filepath.c_str(), nullptr, &shaderSource);
		if (FAILED(hr))
		{
			ARC_CORE_ERROR("Loading shader failed: {}", filepath);
			return;
		}
		ComPtr<IDxcIncludeHandler> includeHandler;
		hr = utils->CreateDefaultIncludeHandler(&includeHandler);
		if (FAILED(hr))
		{
			ARC_CORE_ERROR("Loading shader failed: {}", filepath);
			return;
		}

		ARC_CORE_ASSERT(shaderSource, shaderSource->GetBufferSize());

		const std::vector<const wchar_t*> arguments
		{
#ifdef ARC_DEBUG
			DXC_ARG_DEBUG,
			DXC_ARG_SKIP_OPTIMIZATIONS,
#else
			DXC_ARG_OPTIMIZATION_LEVEL3,
#endif
			DXC_ARG_ALL_RESOURCES_BOUND,
			DXC_ARG_WARNINGS_ARE_ERRORS,
			L"-Qstrip_reflect",
			L"-Qstrip_debug",
		};

		const std::vector<const wchar_t*> defines;


		// Compile shaders
		ComPtr<IDxcBlob> vertexReflection;
		ComPtr<IDxcBlob> pixelReflection;
		ComPtr<IDxcBlob> vertexShader = CompileShader(filepath, shaderSource, includeHandler, ShaderModel::Vertex, arguments, defines, &vertexReflection);
		ComPtr<IDxcBlob> pixelShader = CompileShader(filepath, shaderSource, includeHandler, ShaderModel::Pixel, arguments, defines, &pixelReflection);

		if (!vertexShader || !pixelShader)
		{
			ARC_CORE_ERROR("Failed to compile shader: {}", filepath);
			return;
		}



		_bstr_t shaderName = m_Name.c_str();
		struct Layout
		{
			std::string Name;
			uint32_t Index;
			DXGI_FORMAT Format;
		};
		std::vector<Layout> inputLayout;
		std::vector<Layout> outputLayout;
		std::vector<D3D12_ROOT_PARAMETER> rootParams;
		std::vector<D3D12_DESCRIPTOR_RANGE> rootDescriptors;

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
			AppendMaterials(reflect, rootParams, rootDescriptors, m_MaterialProperties);
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

			AppendMaterials(reflect, rootParams, rootDescriptors, m_MaterialProperties);
		}

		std::vector<D3D12_INPUT_ELEMENT_DESC> psoInputLayout;
		psoInputLayout.reserve(inputLayout.size());
		for (auto& i : inputLayout)
		{
			constexpr auto classification = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			psoInputLayout.emplace_back(i.Name.c_str(), i.Index, i.Format, 0, D3D12_APPEND_ALIGNED_ELEMENT, classification, 0);
		}











		

		// Root Signature /////////////////////////////////////////////////////////////////////
		/*constexpr uint32_t numParameteres = 3;
		CD3DX12_ROOT_PARAMETER parameters[numParameteres];
		parameters[0].InitAsConstants(1, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
		parameters[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		parameters[2].InitAsConstantBufferView(2, 0, D3D12_SHADER_VISIBILITY_VERTEX);

		constexpr uint32_t numSamplers = 1;
		CD3DX12_STATIC_SAMPLER_DESC samplers[1];
		samplers[0].Init(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;

		constexpr auto flags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
			//D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

		rootSigDesc.Init(numParameteres, parameters, numSamplers, samplers, flags);
		*/

		constexpr uint32_t numSamplers = 1;
		CD3DX12_STATIC_SAMPLER_DESC samplers[1];
		samplers[0].Init(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT);
		 
		// Create root signature.
		D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
		rootSigDesc.NumParameters = static_cast<uint32_t>(rootParams.size());
		rootSigDesc.pParameters = rootParams.data();
		rootSigDesc.NumStaticSamplers = numSamplers;
		rootSigDesc.pStaticSamplers = samplers;
		rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
							D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

		ComPtr<ID3DBlob> rootBlob;
		ComPtr<ID3DBlob> errorBlob;
		hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootBlob, &errorBlob);
		if (FAILED(hr))
		{
			if (errorBlob)
				ARC_CORE_ERROR("Failed to serialize Root Signature. Error: {}", static_cast<const char*>(errorBlob->GetBufferPointer()));

			return;
		}

		hr = Dx12Context::GetDevice()->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
		if (FAILED(hr))
		{
			ARC_CORE_ERROR("Failed to create Root Signature. Shader: {}", filepath);
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

		// DepthStencilState
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		psoDesc.DepthStencilState.DepthEnable = TRUE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;

		// InputLayout
		psoDesc.InputLayout.NumElements = static_cast<uint32_t>(psoInputLayout.size());
		psoDesc.InputLayout.pInputElementDescs = psoInputLayout.data();

		psoDesc.SampleMask = 0xFFFFFFFF;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = static_cast<uint32_t>(outputLayout.size());
		for (size_t i = 0; i < outputLayout.size(); ++i)
			psoDesc.RTVFormats[i] = outputLayout[i].Format;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.NodeMask = 0;

		auto* device = Dx12Context::GetDevice();
		hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState));
		if (FAILED(hr))
		{
			ARC_CORE_ERROR("Failed to create Pipeline State. Shader: {}", filepath);
		}

		m_PipelineState->SetName(shaderName);
	}
}
