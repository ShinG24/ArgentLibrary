#include "../Inc/ShaderCompiler.h"

#include <fstream>
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

	void ShaderCompiler::CompileShaderLibrary(LPCWSTR filename, IDxcBlob** pp_blob)
	{
		static Microsoft::WRL::ComPtr<IDxcCompiler> p_compiler = nullptr;
		static Microsoft::WRL::ComPtr<IDxcLibrary> p_library = nullptr;
		static Microsoft::WRL::ComPtr<IDxcIncludeHandler> dxc_include_handler = nullptr;

		if(!p_compiler)
		{
			HRESULT hr{};
			hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(p_compiler.ReleaseAndGetAddressOf()));
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Compiler");
			hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(p_library.ReleaseAndGetAddressOf()));
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Library");
			hr = p_library->CreateIncludeHandler(dxc_include_handler.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Include Handler");
		}

		//Open and read file
		std::ifstream shader_file(filename);
		if(shader_file.good() == false)
		{
			_ASSERT_EXPR(FALSE, L"Failed to read Shader file");
		}

		std::stringstream str_stream;
		str_stream << shader_file.rdbuf();
		std::string s_shader = str_stream.str();

		//Create blob from the string
		Microsoft::WRL::ComPtr<IDxcBlobEncoding> p_tex_blob;
		HRESULT hr = p_library->CreateBlobWithEncodingFromPinned(
			s_shader.c_str(), static_cast<UINT32>(s_shader.size()), 0, p_tex_blob.ReleaseAndGetAddressOf());

		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Shader Blob");

		//Coompile
		Microsoft::WRL::ComPtr<IDxcOperationResult> p_result;
		hr = p_compiler->Compile(p_tex_blob.Get(), filename, 
			L"", L"lib_6_3", nullptr, 0, 
			nullptr, 0, dxc_include_handler.Get(), 
			p_result.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Compile the Shader");

		//Verify the result
		p_result->GetStatus(&hr);
		if(FAILED(hr))
		{
			Microsoft::WRL::ComPtr<IDxcBlobEncoding> p_error;
			hr = p_result->GetErrorBuffer(p_error.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Get Shader compiler error");

			//Convert error blob to a string
			std::vector<char> info_log(p_error->GetBufferSize() + 1);
			memcpy(info_log.data(), p_error->GetBufferPointer(),
				p_error->GetBufferSize());

			info_log[p_error->GetBufferSize()] = 0;

			std::string error_msg = "Shader Compiler Error: \n";
			error_msg.append(info_log.data());

			MessageBoxA(nullptr, error_msg.c_str(), "Error!", MB_OK);
		}
		hr = p_result->GetResult(pp_blob);
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Get Result");
	}
}
