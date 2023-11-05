#pragma once

#include <wrl.h>

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

		Shader();
		~Shader() = default;

		Shader(const Shader&) = delete;
		Shader(const Shader&&) = delete;
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(const Shader&&) = delete;


		void* GetData() const { return dxc_blob_->GetBufferPointer(); }
		size_t GetSize() const { return dxc_blob_->GetBufferSize(); }
		Type GetType() const { return type_; }

		D3D12_SHADER_BYTECODE GetShaderByteCode() const { return shader_byte_code_; }

	private:

		Type type_;
		Microsoft::WRL::ComPtr<IDxcBlob> dxc_blob_;
		D3D12_SHADER_BYTECODE shader_byte_code_;

	};
}


