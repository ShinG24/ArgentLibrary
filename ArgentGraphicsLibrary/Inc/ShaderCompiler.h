#pragma once

#include <wrl.h>
#include "../External/DXC/Inc/dxcapi.h"

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

	private:
		Microsoft::WRL::ComPtr<IDxcCompiler3> dxc_compiler{};
		Microsoft::WRL::ComPtr<IDxcIncludeHandler> dxc_include_handler{};
		Microsoft::WRL::ComPtr<IDxcUtils> dxc_utils{};

	};
}