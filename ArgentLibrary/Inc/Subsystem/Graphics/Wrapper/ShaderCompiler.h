#pragma once

#include <wrl.h>

#include <dxcapi.h>
#include <d3d12shader.h>

namespace argent::graphics
{
	class ShaderCompiler
	{
	public:
		ShaderCompiler();
		~ShaderCompiler() = default;


		ShaderCompiler(ShaderCompiler&) = delete;
		ShaderCompiler(ShaderCompiler&&) = delete;
		ShaderCompiler& operator=(ShaderCompiler&) = delete;
		ShaderCompiler& operator=(ShaderCompiler&&) = delete;

		void Compile(const wchar_t* filename, LPCWSTR target_profile, 
		IDxcBlob** pp_blob) const;

		void CompileShaderLibrary(LPCWSTR filename, IDxcBlob** pp_blob) const;

		void CompileShaderLibraryWithReflectionData(LPCWSTR filename);

		void CompileAndCreateShaderReflection(const wchar_t* filepath, LPCWSTR target_profile, 
			IDxcBlob** pp_blob, ID3D12ShaderReflection** pp_shader_reflection);

	private:
		Microsoft::WRL::ComPtr<IDxcCompiler3> dxc_compiler{};
		Microsoft::WRL::ComPtr<IDxcIncludeHandler> dxc_include_handler{};
		Microsoft::WRL::ComPtr<IDxcUtils> dxc_utils{};

	};
}