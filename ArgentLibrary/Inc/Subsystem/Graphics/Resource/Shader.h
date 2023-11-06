#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <d3d12shader.h>
#include <dxcapi.h>


namespace argent::graphics
{
	class Shader
	{
	public:
		enum Type
		{
			Vertex,
			Pixel,
			Domain,
			Geometry,
			Hull,
			Compute,
			Mesh,
			Amplification,
		//	Library,
			Max
		};

	public:

		Shader(const char* filepath, Type type);
		~Shader() = default;

		Shader(const Shader&) = delete;
		Shader(const Shader&&) = delete;
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(const Shader&&) = delete;


		void* GetData() const { return dxc_blob_->GetBufferPointer(); }
		size_t GetSize() const { return dxc_blob_->GetBufferSize(); }
		Type GetType() const { return type_; }

		D3D12_SHADER_BYTECODE GetShaderByteCode() const { return shader_byte_code_; }

		ID3D12ShaderReflection* GetShaderReflection() const { return shader_reflection_.Get(); }

	private:

		Type type_;
		Microsoft::WRL::ComPtr<IDxcBlob> dxc_blob_;
		Microsoft::WRL::ComPtr<ID3D12ShaderReflection> shader_reflection_;
		D3D12_SHADER_BYTECODE shader_byte_code_;

	};
}


