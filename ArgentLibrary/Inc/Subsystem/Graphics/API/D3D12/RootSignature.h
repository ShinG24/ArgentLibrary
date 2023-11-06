#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <vector>

//TODO Make RootSignature wrap class
namespace argent::graphics::dx12
{
	class GraphicsDevice;

	/**
	 * \brief ID3D12RootSignature のラップクラス
	 * Add~()関数を使ってパラメータ、レンジを追加し、Create()で実際のオブジェクトを作成
	 */
	class RootSignature
	{
	public:

		RootSignature() = default;
		~RootSignature() = default;

		RootSignature(RootSignature&) = delete;
		RootSignature(RootSignature&&) = delete;
		RootSignature& operator=(RootSignature&) = delete;
		RootSignature& operator=(RootSignature&&) = delete;

	private:

		enum
		{
			BaseShaderRegister = 0,
			NumDescriptors = 1,
			RegisterSpace = 2,
			RangeType = 3,
		};

	public:

		/**
		 * \brief ディスクリプタテーブル追加関数
		 * d3d12descriptorrange が1つですむ場合に使う
		 * \param base_shader_register シェーダーレジスタスタート
		 * \param range_type range type
		 * \param num_descriptors ディスクリプタの数
		 * \param register_space レジスタスペース
		 * \param offset_in_descriptors_from_table_start 入力の必要なし 
		 */
		void AddHeapRangeParameter(UINT base_shader_register, UINT num_descriptors,
			D3D12_DESCRIPTOR_RANGE_TYPE range_type,
			UINT register_space, UINT offset_in_descriptors_from_table_start = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);

		/**
		 * \brief 複数個のrangeがある場合のディスクリプタテーブル追加関数
		 * \param ranges ranges
		 */
		void AddHeapRangeParameters(std::vector<std::tuple<UINT, //BaseShaderRegister
		                                                   UINT, //NumDescriptors
		                                                   UINT, //RegisterSpace
		                                                   D3D12_DESCRIPTOR_RANGE_TYPE	//RangeType
		>> ranges);

		/**
		 * \brief 複数のrangeがある場合のディスクリプタテーブル追加関数
		 * \param ranges ranges
		 */
		void AddHeapRangeParameters(const std::vector<D3D12_DESCRIPTOR_RANGE>& ranges);

		void AddRootParameter(D3D12_ROOT_PARAMETER_TYPE type, UINT shader_register, 
			UINT register_space, UINT num_root_constants);

		/**
		 * \brief RootSignatureObject生成関数
		 * \param graphics_device GraphicsDeviceのポインタ
		 * \param flag RootSignature Flag
		 */
		void Create(const GraphicsDevice* graphics_device, D3D12_ROOT_SIGNATURE_FLAGS flag);

		/**
		 * \brief RootSignatureオブジェクトを取得
		 * \return ID3D12RootSignature
		 */
		ID3D12RootSignature* GetRootSignatureObject() const { return root_signature_object_.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_object_;

		//データ保持用
		std::vector<D3D12_ROOT_PARAMETER> root_parameters_;
		std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> descriptor_ranges_;
		std::vector<UINT> range_locations_;

	};
}