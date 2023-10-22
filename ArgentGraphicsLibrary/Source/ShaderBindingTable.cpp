#include"../Inc/ShaderBindingTable.h"

#include "../Inc/GraphicsCommon.h"
#include "../Inc/GraphicsDevice.h"

namespace argent::graphics
{
	UINT ShaderBindingTable::AddShaderIdentifier(const std::wstring& shader_identifier)
	{
		const UINT index = static_cast<UINT>(shader_identifier_.size());
		shader_identifier_.emplace_back(shader_identifier);
		return index;
	}

	UINT ShaderBindingTable::AddShaderIdentifierAndInputData(const std::wstring& shader_identifier,
		const std::vector<void*>& data)
	{
		AddInputData(data);
		return AddShaderIdentifier(shader_identifier);
	}

	void ShaderBindingTable::Generate(const GraphicsDevice* graphics_device, ID3D12StateObjectProperties* state_object_properties)
	{
		if (shader_identifier_.size() != input_data_.size() && input_data_.size() != 0) _ASSERT_EXPR(FALSE, L"Shader Identifier size and Input Data size need to be same value");

		const UINT input_data_size = sizeof(uint8_t) * (input_data_.size() == 0) ? 0 : static_cast<UINT>(input_data_.at(0).size());
		stride_ = _ALIGNMENT_(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + input_data_size, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		const UINT buffer_size = _ALIGNMENT_(static_cast<UINT>(stride_ * shader_identifier_.size()), D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		graphics_device->CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, buffer_size, 
			D3D12_RESOURCE_STATE_GENERIC_READ, resource_object_.ReleaseAndGetAddressOf());

		CopyToBuffer(state_object_properties);
	}

	void ShaderBindingTable::CopyToBuffer(ID3D12StateObjectProperties* state_object_properties)
	{
		uint8_t* map;
		resource_object_->Map(0u, nullptr, reinterpret_cast<void**>(&map));

		for(size_t i = 0; i < shader_identifier_.size(); ++i)
		{
			void* id = state_object_properties->GetShaderIdentifier(shader_identifier_.at(i).c_str());
			memcpy(map, id, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			map += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

			//TODO Maybe some
			if(input_data_.size() != 0)
			{
				memcpy(map, input_data_.at(i).data(), input_data_.at(i).size() * sizeof(uint8_t));
			}
			map += stride_ - D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		}

		resource_object_->Unmap(0u, nullptr);
	}

}
