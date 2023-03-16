#include <arcpch.h>
#include "Dx12Shader.h"

#include "d3dx12.h"

#include <dxc/inc/dxcapi.h>
#include <dxc/inc/d3d12shader.h>
#include <wrl.h>

namespace ArcEngine
{
	Dx12Shader::Dx12Shader(const std::filesystem::path& filepath, const BufferLayout& layout)
	{
		ARC_PROFILE_SCOPE()

		//SetInputLayoutFromBufferLayout(layout);
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

	void Dx12Shader::SetInt(const std::string& name, int value)
	{
	}

	void Dx12Shader::SetIntArray(const std::string& name, const int* values, uint32_t count)
	{
	}

	void Dx12Shader::SetFloat(const std::string& name, float value)
	{
	}

	void Dx12Shader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
	}

	void Dx12Shader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
	}

	void Dx12Shader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
	}

	void Dx12Shader::SetMat3(const std::string& name, const glm::mat3& value)
	{
	}

	void Dx12Shader::SetMat4(const std::string& name, const glm::mat4& value)
	{
	}

	void Dx12Shader::SetData(uint32_t slot, uint32_t num32BitValues, void* data)
	{
		Dx12Context::GetGraphicsCommandList()->SetGraphicsRoot32BitConstants(slot, num32BitValues, data, 0);
	}

	std::unordered_map<std::string, MaterialProperty, UM_StringTransparentEquality>& Dx12Shader::GetMaterialProperties()
	{
		std::unordered_map<std::string, MaterialProperty, UM_StringTransparentEquality> map;
		return map;
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
			if (componentType == D3D_REGISTER_COMPONENT_UINT32)				return DXGI_FORMAT_R32_UINT;
			else if (componentType == D3D_REGISTER_COMPONENT_SINT32)		return DXGI_FORMAT_R32_SINT;
			else if (componentType == D3D_REGISTER_COMPONENT_FLOAT32)		return DXGI_FORMAT_R32_FLOAT;
		}
		else if (mask <= 3)
		{
			if (componentType == D3D_REGISTER_COMPONENT_UINT32)				return DXGI_FORMAT_R32G32_UINT;
			else if (componentType == D3D_REGISTER_COMPONENT_SINT32)		return DXGI_FORMAT_R32G32_SINT;
			else if (componentType == D3D_REGISTER_COMPONENT_FLOAT32)		return DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (mask <= 7)
		{
			if (componentType == D3D_REGISTER_COMPONENT_UINT32)				return DXGI_FORMAT_R32G32B32_UINT;
			else if (componentType == D3D_REGISTER_COMPONENT_SINT32)		return DXGI_FORMAT_R32G32B32_SINT;
			else if (componentType == D3D_REGISTER_COMPONENT_FLOAT32)		return DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (mask <= 15)
		{
			if (componentType == D3D_REGISTER_COMPONENT_UINT32)				return DXGI_FORMAT_R32G32B32A32_UINT;
			else if (componentType == D3D_REGISTER_COMPONENT_SINT32)		return DXGI_FORMAT_R32G32B32A32_SINT;
			else if (componentType == D3D_REGISTER_COMPONENT_FLOAT32)		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		return DXGI_FORMAT_UNKNOWN;
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




		struct Layout
		{
			std::string Name;
			uint32_t Index;
			DXGI_FORMAT Format;
		};
		std::vector<Layout> inputLayout;
		std::vector<Layout> outputLayout;


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

#if 0
			ARC_CORE_DEBUG("======================= CB ================================");
			for (uint32_t i = 0; i < shaderDesc.ConstantBuffers; ++i)
			{
				D3D12_SHADER_BUFFER_DESC desc;
				reflect->GetConstantBufferByIndex(i)->GetDesc(&desc);
				ARC_CORE_DEBUG("\tName: {}", desc.Name);
				ARC_CORE_DEBUG("\tSize: {}", desc.Size);
				ARC_CORE_DEBUG("\tType: {}", desc.Type);
				ARC_CORE_DEBUG("\tVariables: {}", desc.Variables);
			}
			ARC_CORE_DEBUG("======================= Resources ================================");
			for (uint32_t i = 0; i < shaderDesc.BoundResources; ++i)
			{
				D3D12_SHADER_INPUT_BIND_DESC desc;
				reflect->GetResourceBindingDesc(i, &desc);
				ARC_CORE_DEBUG("\tName: {}", desc.Name);
				ARC_CORE_DEBUG("\tBindPoint: {}", desc.BindPoint);
				ARC_CORE_DEBUG("\tBindCount: {}", desc.BindCount);
				ARC_CORE_DEBUG("\tType: {}", desc.Type);
				ARC_CORE_DEBUG("\tSpace: {}", desc.Space);
			}
#endif
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
#if 0
			ARC_CORE_DEBUG("======================= CB ================================");
			for (uint32_t i = 0; i < shaderDesc.ConstantBuffers; ++i)
			{
				D3D12_SHADER_BUFFER_DESC desc;
				reflect->GetConstantBufferByIndex(i)->GetDesc(&desc);
				ARC_CORE_DEBUG("\tName: {}", desc.Name);
				ARC_CORE_DEBUG("\tSize: {}", desc.Size);
				ARC_CORE_DEBUG("\tType: {}", desc.Type);
				ARC_CORE_DEBUG("\tVariables: {}", desc.Variables);
			}
			ARC_CORE_DEBUG("======================= Resources ================================");
			for (uint32_t i = 0; i < shaderDesc.BoundResources; ++i)
			{
				D3D12_SHADER_INPUT_BIND_DESC desc;
				reflect->GetResourceBindingDesc(i, &desc);
				ARC_CORE_DEBUG("\tName: {}", desc.Name);
				ARC_CORE_DEBUG("\tBindPoint: {}", desc.BindPoint);
				ARC_CORE_DEBUG("\tBindCount: {}", desc.BindCount);
				ARC_CORE_DEBUG("\tType: {}", desc.Type);
				ARC_CORE_DEBUG("\tSpace: {}", desc.Space);
			}
#endif
		}

		std::vector<D3D12_INPUT_ELEMENT_DESC> psoInputLayout;
		psoInputLayout.reserve(inputLayout.size());
		for (auto& i : inputLayout)
		{
			constexpr auto classification = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			psoInputLayout.emplace_back(i.Name.c_str(), i.Index, i.Format, 0, D3D12_APPEND_ALIGNED_ELEMENT, classification, 0);
		}











		

		// Root Signature /////////////////////////////////////////////////////////////////////
		constexpr uint32_t numParameteres = 3;
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

		ComPtr<ID3DBlob> rootBlob;
		ComPtr<ID3DBlob> errorBlob;
		hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob);
		if (FAILED(hr))
		{
			if (errorBlob)
				ARC_CORE_ERROR("Failed to serialize Root Signature. Error: {}", errorBlob->GetBufferPointer());

			return;
		}

		hr = Dx12Context::GetDevice()->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
		if (FAILED(hr))
		{
			ARC_CORE_ERROR("Failed to create Root Signature. Shader: {}", filepath);
			return;
		}

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
	}
}
