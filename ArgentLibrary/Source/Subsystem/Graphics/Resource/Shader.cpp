#include "Subsystem/Graphics/Resource/Shader.h"

#include <filesystem>

#include "Subsystem/Graphics/Wrapper/ShaderCompiler.h"


namespace argent::graphics
{

	Shader::Shader(const char* filepath, Type type)
	{
		LPCWSTR target_profile{};
		switch (type)
		{
		case Vertex:
			target_profile = L"vs_6_3";
			break;
		case Pixel:
			target_profile = L"ps_6_3";
			break;
		case Geometry:
			target_profile = L"gs_6_3";
			break;
		case Hull:
			target_profile = L"hs_6_3";
			break;
		case Domain:
			target_profile = L"ds_6_3";
			break;
		case Compute:
			target_profile = L"cs_6_3";
			break;
		}

				ShaderCompiler compiler;
		std::filesystem::path path = filepath;
		compiler.CompileAndCreateShaderReflection(path.wstring().c_str(), 
			target_profile, dxc_blob_.ReleaseAndGetAddressOf(), 
			shader_reflection_.ReleaseAndGetAddressOf());

		shader_byte_code_.BytecodeLength = dxc_blob_->GetBufferSize();
		shader_byte_code_.pShaderBytecode = dxc_blob_->GetBufferPointer();

	}
}
