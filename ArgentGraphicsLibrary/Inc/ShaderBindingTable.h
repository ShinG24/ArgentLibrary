#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <string>
#include <vector>

namespace argent::graphics
{
	class GraphicsDevice;
	
	class ShaderBindingTable
	{
	public:
		ShaderBindingTable() = default;
		~ShaderBindingTable() = default;

		UINT AddShaderIdentifier(const std::wstring& shader_identifier);
		UINT AddShaderIdentifierAndInputData(const std::wstring& shader_identifier, const std::vector<void*>& data);
		void AddInputData(const std::vector<void*>& data) { input_data_.emplace_back(data); }

		void Generate(const GraphicsDevice* graphics_device, ID3D12StateObjectProperties* state_object_properties);

		void CopyToBuffer(ID3D12StateObjectProperties* state_object_properties);

		D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return resource_object_->GetGPUVirtualAddress(); }
		UINT GetSize() const { return static_cast<UINT>(resource_object_->GetDesc().Width); }
		UINT GetStride() const { return stride_; }

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_object_;
		std::vector<std::vector<void*>> input_data_;
		std::vector<std::wstring> shader_identifier_;
		UINT stride_;
	};
}