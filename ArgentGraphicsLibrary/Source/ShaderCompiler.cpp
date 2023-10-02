#include "../Inc/ShaderCompiler.h"

#include <filesystem>
#include <vector>

namespace argent::graphics
{
	ShaderCompiler::ShaderCompiler()
	{
		HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(dxc_utils.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create DXC UTils");

		hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(dxc_compiler.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create DXC Compiler");

		hr = dxc_utils->CreateDefaultIncludeHandler(dxc_include_handler.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create DXC Include Handler");
	}

	void ShaderCompiler::Compile(const wchar_t* filename, LPCWSTR target_profile, IDxcBlob** pp_blob) const
	{

		const LPCWSTR entry_point = L"main";
		std::vector<LPCWSTR> compilation_arguments
		{
			L"-E",
			entry_point,
			L"-T",
			target_profile,
			DXC_ARG_PACK_MATRIX_ROW_MAJOR,
			DXC_ARG_WARNINGS_ARE_ERRORS,
			DXC_ARG_ALL_RESOURCES_BOUND,

#ifdef _DEBUG
			DXC_ARG_DEBUG,
#endif
			L"-I",				//To specify include directory
			L"./Assets/Shader",
		};

		Microsoft::WRL::ComPtr<IDxcBlobEncoding> source_blob{};
		HRESULT hr = dxc_utils->LoadFile(filename, nullptr, source_blob.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Load Shader");

		DxcBuffer source_buffer
		{
			.Ptr = source_blob->GetBufferPointer(),
			.Size = source_blob->GetBufferSize(),
			.Encoding = 0u,
		};


		//ƒRƒ“ƒpƒCƒ‹
		Microsoft::WRL::ComPtr<IDxcResult> result;
		hr = dxc_compiler->Compile(&source_buffer,
			compilation_arguments.data(),
			static_cast<uint32_t>(compilation_arguments.size()),
			dxc_include_handler.Get(), IID_PPV_ARGS(result.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Compile Shader");

		Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors{};
		hr = result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
		_ASSERT_EXPR(SUCCEEDED(hr),L"Failed to Compile Shader");
		if (errors.Get() && errors->GetStringLength() > 0)
		{
			std::filesystem::path path2{ errors->GetStringPointer() };
			_ASSERT_EXPR(false, path2.wstring().c_str());
		}

		result->GetResult(pp_blob);
	}

}
