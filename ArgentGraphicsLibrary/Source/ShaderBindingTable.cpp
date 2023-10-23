#include"../Inc/ShaderBindingTable.h"

#include "../Inc/GraphicsCommon.h"
#include "../Inc/GraphicsDevice.h"

namespace argent::graphics
{
	UINT ShaderBindingTable::AddShaderIdentifierAndInputData(const std::wstring& shader_identifier,
		const std::vector<void*>& data)
	{
		if(shader_identifier.empty()) _ASSERT_EXPR(FALSE, L"Shader Identifier can not be null");
		const UINT index = static_cast<UINT>(shader_tables_.size());
		ShaderTable shader_table;
		shader_table.shader_identifier_ = shader_identifier.c_str();
		shader_table.input_data_ = data;
		shader_tables_.emplace_back(shader_table);
		return index;
	}

	UINT ShaderBindingTable::AddShaderIdentifier(const std::wstring& shader_identifier)
	{
		if(shader_identifier.empty()) _ASSERT_EXPR(FALSE, L"Shader Identifier can not be null");
		const UINT index = static_cast<UINT>(shader_tables_.size());
		ShaderTable shader_table;
		shader_table.shader_identifier_ = shader_identifier;
		shader_tables_.emplace_back(shader_table);
		return index;
	}

	void ShaderBindingTable::Generate(const GraphicsDevice* graphics_device, ID3D12StateObjectProperties* state_object_properties)
	{
		if (shader_tables_.empty()) _ASSERT_EXPR(FALSE, L"Shader Identifier size and Input Data size need to be same value");

		UINT max_root_args = 0u;
		for(const auto& table : shader_tables_)
		{
			if(table.input_data_.size() > max_root_args)
				max_root_args = static_cast<UINT>(table.input_data_.size());
		}
		const UINT input_data_size = sizeof(uint64_t) * max_root_args;
		entry_size_ = _ALIGNMENT_(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + input_data_size, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		const UINT buffer_size = static_cast<UINT>(entry_size_ * shader_tables_.size());

		graphics_device->CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, buffer_size, 
			D3D12_RESOURCE_STATE_GENERIC_READ, resource_object_.ReleaseAndGetAddressOf());

		CopyToBuffer(state_object_properties);
	}

	void ShaderBindingTable::CopyToBuffer(ID3D12StateObjectProperties* state_object_properties)
	{
		uint8_t* map;
		resource_object_->Map(0u, nullptr, reinterpret_cast<void**>(&map));

		for(const auto& table : shader_tables_)
		{
			void* id = state_object_properties->GetShaderIdentifier(table.shader_identifier_.c_str());
			memcpy(map, id, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			map += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

			//TODO Maybe some
			if(!table.input_data_.empty())
			{
				memcpy(map, table.input_data_.data(), sizeof(uint64_t) * table.input_data_.size());
			}
			map += entry_size_ - D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		}

		resource_object_->Unmap(0u, nullptr);
	}

}
