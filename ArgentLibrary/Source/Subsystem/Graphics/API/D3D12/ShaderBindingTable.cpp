#include "Subsystem/Graphics/API/D3D12/ShaderBindingTable.h"

#include "Subsystem/Graphics/API/D3D12/D3D12Common.h"
#include "Subsystem/Graphics/API/D3D12/GraphicsDevice.h"

#include "Subsystem/Graphics/Common/GraphicsCommon.h"

namespace argent::graphics::dx12
{
	UINT ShaderBindingTable::AddShaderIdentifierAndInputData(const std::wstring& shader_identifier,
		const std::vector<void*>& data)
	{
		if(shader_identifier.empty()) _ASSERT_EXPR(FALSE, L"Shader Identifier can not be null");
		const UINT index = static_cast<UINT>(shader_tables_.size());
		ShaderTable shader_table;
		shader_table.shader_identifier_ = shader_identifier;
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

	UINT ShaderBindingTable::AddShaderTable(const ShaderTable& shader_table)
	{
		if(shader_table.shader_identifier_.empty()) _ASSERT_EXPR(FALSE, L"Shader Identifier can not be null");
		const UINT index = static_cast<UINT>(shader_tables_.size());
		shader_tables_.emplace_back(shader_table);
		return index;
	}

	void ShaderBindingTable::AddShaderTables(const std::vector<ShaderTable>& shader_tables)
	{
		for(const auto& table : shader_tables)
		{
			if(table.shader_identifier_.empty()) _ASSERT_EXPR(FALSE, L"Shader Identifier can not be null");

			shader_tables_.emplace_back(table);
		}
	}

	//Bufferの作成
	void ShaderBindingTable::Generate(const GraphicsDevice* graphics_device,
	                                  ID3D12StateObjectProperties* state_object_properties, LPCWSTR resource_object_name)
	{
		//ShaderTableがNullの場合は警告
		if (shader_tables_.empty()) _ASSERT_EXPR(FALSE, L"Shader Identifier size and Input Data size need to be same value");

		//バインドするリソース数の最大数を計算
		UINT max_root_args = 0u;
		for(const auto& table : shader_tables_)
		{
			if(table.input_data_.size() > max_root_args)
				max_root_args = static_cast<UINT>(table.input_data_.size());
		}

		//バインドするデータはGpuPointer == UINT64型
		//実際に必要なサイズを計算する
		const UINT input_data_size = sizeof(uint64_t) * max_root_args;

		//データは32bitアライメント id == Shader Indentifierに必要なサイズ
		entry_size_ = _ALIGNMENT_(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + input_data_size, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		//バッファのサイズはShaderTableの数 * データサイズ
		const UINT buffer_size = static_cast<UINT>(entry_size_ * shader_tables_.size());

		graphics_device->CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, buffer_size, 
			D3D12_RESOURCE_STATE_GENERIC_READ, resource_object_.ReleaseAndGetAddressOf());

		resource_object_->SetName(resource_object_name);
		//リソースのコピー
		CopyToBuffer(state_object_properties);
	}

	void ShaderBindingTable::CopyToBuffer(ID3D12StateObjectProperties* state_object_properties)
	{
		//1バイト単位でバインドするため
		uint8_t* map;
		resource_object_->Map(0u, nullptr, reinterpret_cast<void**>(&map));

		for(const auto& table : shader_tables_)
		{
			//Shader Identifierのバインド
			void* id = state_object_properties->GetShaderIdentifier(table.shader_identifier_.c_str());
			memcpy(map, id, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			map += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

			//Tableにデータが入っていた場合のみバインドする
			if(!table.input_data_.empty())
			{
				memcpy(map, table.input_data_.data(), sizeof(uint64_t) * table.input_data_.size());
			}
			map += entry_size_ - D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		}

		resource_object_->Unmap(0u, nullptr);
	}

}
