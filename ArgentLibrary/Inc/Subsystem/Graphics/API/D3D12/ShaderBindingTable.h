#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <string>
#include <vector>

namespace argent::graphics::dx12
{
	class GraphicsDevice;
}

namespace argent::graphics::dx12
{
	struct ShaderTable
	{
		//Raygen Missの場合は普通に関数名、
		//Hitの場合はHitGroupName
		std::wstring shader_identifier_;
		//Inputデータ、ルートシグネチャと紐づいているので
		//RootParameterのIndexに合わせてバインドすればいいと思う
		//基本的にBufferのGpuAddressかDescriptorのGpuHandle
		std::vector<void*> input_data_;
	};

	/**
	 * \brief Shader TableをWrapしたクラス
	 * バインドするためのバッファとバインドする情報を保持
	 * Shader Identifierの数だけバインドする変数が必要になる
	 */
	class ShaderBindingTable
	{
	public:
		ShaderBindingTable() = default;
		~ShaderBindingTable() = default;

		ShaderBindingTable(const ShaderBindingTable&) = delete;
		ShaderBindingTable(const ShaderBindingTable&&) = delete;
		ShaderBindingTable& operator=(const ShaderBindingTable&) = delete;
		ShaderBindingTable& operator=(const ShaderBindingTable&&) = delete;

		/**
		 * \brief シェーダーのシンボルとバインドするデータを追加
		 * \param shader_identifier Shader Identifier Raygen or Miss = 関数名 HitGroup = Hit Group Name
		 * \param data Data ルートシグネチャをみて合ってるか確認する
		 * \return インデックス
		 */
		UINT AddShaderIdentifierAndInputData(const std::wstring& shader_identifier, const std::vector<void*>& data);

		/**
		 * \brief シェーダーシンボルの追加
		 * \param shader_identifier shader_identifier Shader Identifier Raygen or Miss = 関数名 HitGroup = Hit Group Name
		 * \return インデックス
		 */
		UINT AddShaderIdentifier(const std::wstring& shader_identifier);

		/**
		 * \brief シェーダーテーブルの追加
		 * \param shader_table Shader Table
		 * \return Index
		 */
		UINT AddShaderTable(const ShaderTable& shader_table);

		/**
		 * \brief 複数のシェーダーテーブル追加
		 * \param shader_tables Shader Table
		 */
		void AddShaderTables(const std::vector<ShaderTable>& shader_tables);

		/**
		 * \brief 与えられた情報からシェーダーテーブルオブジェクトを作成
		 * \param graphics_device Graphics Device
		 * \param state_object_properties State Object Properties idを取得するために
		 * \param resource_object_name Bufferの名前
		 */
		void Generate(const GraphicsDevice* graphics_device, ID3D12StateObjectProperties* state_object_properties,
		              LPCWSTR resource_object_name);

		/**
		 * \brief バッファへ情報をコピーする
		 * \param state_object_properties ID3D12StateObjectProperties
		 */
		void CopyToBuffer(ID3D12StateObjectProperties* state_object_properties);

		/**
		 * \brief Resource Objectのポインタ
		 * \return D3D12_GPU_VIRTUAL_ADDRESS
		 */
		D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return resource_object_->GetGPUVirtualAddress(); }

		/**
		 * \brief バッファのサイズ
		 * \return Buffer Size
		 */
		UINT GetSize() const { return static_cast<UINT>(resource_object_->GetDesc().Width); }

		/**
		 * \brief Bufferの区切りサイズ == EntrySize
		 * \return Stride Size
		 */
		UINT GetStride() const { return entry_size_; }

	private:

		std::vector<ShaderTable> shader_tables_{};					//情報まとめ
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_object_{};	//実際に情報を保持しているバッファ
																	//描画するときにセットする
		UINT entry_size_{};	//ShaderIdentifier一個とそれに紐づいた情報をバインドするのに必要なサイズ
							//Bufferのサイズではないので注意
							//Bufferのサイズ==EntrySize * ShaderTables.size()

	};
}