#include "arcpch.h"
#include "Dx12Shader.h"

#include "d3dx12.h"

#include <comutil.h>
#include <dxc/inc/dxcapi.h>
#include <wrl.h>

#include <ranges>

#include "Arc/Core/Filesystem.h"

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

		for (auto& shader : m_ShaderBlobs | std::views::values)
			shader->Release();

		for (auto& reflection : m_ReflectionBlobs | std::views::values)
			reflection->Release();
	}

	void Dx12Shader::Recompile(const std::filesystem::path& path)
	{
		ARC_PROFILE_SCOPE()

		for (auto& shader : m_ShaderBlobs | std::views::values)
			shader->Release();

		for (auto& reflection : m_ReflectionBlobs | std::views::values)
			reflection->Release();

		Compile(path);
	}

	const std::string& Dx12Shader::GetName() const
	{
		return m_Name;
	}

	inline static std::map<ShaderType, const wchar_t*> s_TargetMap
	{
		{ ShaderType::Vertex,		L"vs_6_6" },
		{ ShaderType::Pixel,		L"ps_6_6" },
	};

	inline static std::map<ShaderType, const wchar_t*> s_EntryPointMap
	{
		{ ShaderType::Vertex,		L"VS_Main" },
		{ ShaderType::Pixel,		L"PS_Main" },
	};

	static IDxcBlob* CompileShader(
		const std::filesystem::path& filepath,
		const Microsoft::WRL::ComPtr<IDxcBlobEncoding>& encodedBlob,
		const Microsoft::WRL::ComPtr<IDxcIncludeHandler>& includeHandler,
		ShaderType shaderModel,
		const std::vector<const wchar_t*>& arguments,
		const std::vector<const wchar_t*>& defines,
		IDxcBlob** reflection)
	{
		ARC_PROFILE_SCOPE()

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
		std::wstring pdbParentPath = filepath.parent_path().c_str();
		pdbParentPath += L"/Debug/";
		std::wstring pdbName = std::wstring(filepath.filename().stem());
		pdbName += L"_";
		pdbName += s_TargetMap.at(shaderModel);
		pdbName += L".pdb";
		const std::filesystem::path pdbPath = pdbParentPath + pdbName;
		std::vector args
		{
			filepath.filename().c_str(),
			L"-E", s_EntryPointMap.at(shaderModel),
			L"-T", s_TargetMap.at(shaderModel),
			L"-Fd", pdbPath.c_str()
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
			compileResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(reflection), nullptr);

		// Write PDB for PIX
		{
			ComPtr<IDxcBlob> pdb = nullptr;
			ComPtr<IDxcBlobUtf16> pPDBName = nullptr;
			hr = compileResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pdb), &pPDBName);
			if (SUCCEEDED(hr))
			{
				ScopedBuffer buffer(pdb->GetBufferSize());
				memcpy(buffer.Data(), pdb->GetBufferPointer(), buffer.Size());
				if (!Filesystem::WriteFileBinary(pPDBName->GetStringPointer(), buffer))
					ARC_CORE_ERROR("Failed to write PDB to disk: {}", pdbPath);
			}
			else
			{
				ARC_CORE_ERROR("Failed to create PDB for {}", filepath);
			}
		}

		IDxcBlob* shader;
		compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr);
		return shader;
	}

	void Dx12Shader::Compile(const std::filesystem::path& filepath)
	{
		ARC_PROFILE_SCOPE()

		using namespace Microsoft::WRL;
		
		ComPtr<IDxcUtils> utils = nullptr;
		HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
		if (FAILED(hr))
		{
			ARC_CORE_ERROR("DxcUtils creation failed!");
			return;
		}

		ComPtr<IDxcBlobEncoding> source;
		hr = utils->LoadFile(filepath.c_str(), nullptr, &source);
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

		ARC_CORE_ASSERT(source, source->GetBufferSize());

		const std::vector<const wchar_t*> args
		{
#ifdef ARC_DEBUG
			DXC_ARG_DEBUG,
			DXC_ARG_SKIP_OPTIMIZATIONS,
#else
			DXC_ARG_OPTIMIZATION_LEVEL3,
#endif
#ifdef ARC_DIST
			L"-Qstrip_reflect",
			L"-Qstrip_debug",
#endif
			DXC_ARG_ALL_RESOURCES_BOUND,
			DXC_ARG_WARNINGS_ARE_ERRORS,
			L"-I", L"assets/shaders",
		};

		const std::vector<const wchar_t*> defs;

		// Compile shaders
		IDxcBlob* vertReflection = nullptr;
		IDxcBlob* pixelReflection = nullptr;
		auto* vertexShader = CompileShader(filepath, source, includeHandler, ShaderType::Vertex, args, defs, &vertReflection);
		auto* pixelShader = CompileShader(filepath, source, includeHandler, ShaderType::Pixel, args, defs, &pixelReflection);
		
		if (!vertexShader || !pixelShader)
		{
			ARC_CORE_ERROR("Failed to compile shader: {}", filepath);
			return;
		}

		m_ShaderBlobs[ShaderType::Vertex] = vertexShader;
		m_ShaderBlobs[ShaderType::Pixel] = pixelShader;
		m_ReflectionBlobs[ShaderType::Vertex] = vertReflection;
		m_ReflectionBlobs[ShaderType::Pixel] = pixelReflection;
	}
}
