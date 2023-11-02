#pragma once

#include <d3d12.h>
#include <string>
#include <vector>
#include <wrl.h>

#include "../External/DXC/Inc/dxcapi.h"

namespace argent::graphics::dx12
{
	class GraphicsDevice;
}

namespace argent::graphics::dx12
{
	/**
	 * \brief DXR用のパイプランステートクラス
	 * 複数のサブオブジェクトのまとまりからなる
	 */
	class RaytracingPipelineState
	{
	public:

		RaytracingPipelineState() = default;
		~RaytracingPipelineState() = default;

		RaytracingPipelineState(const RaytracingPipelineState&) = delete;
		RaytracingPipelineState(const RaytracingPipelineState&&) = delete;
		RaytracingPipelineState& operator=(const RaytracingPipelineState&) = delete;
		RaytracingPipelineState& operator=(const RaytracingPipelineState&&) = delete;

		/**
		 * \brief Shader Libraryの追加
		 * シェーダーオブジェクトへのポインタとシンボル付きの関数名を紐付ける
		 * \param p_dxc_blob Compiled Shader Pointer
		 * \param export_symbol Export Symbol
		 */
		void AddLibrary(IDxcBlob* p_dxc_blob, const std::vector<std::wstring>& export_symbol);

		/**
		 * \brief ヒットグループの追加
		 * \param hit_group_name Hit Group Name 任意の名前、ただしユニークなもの
		 * \param closest_hit Closest Hit Shader Name 
		 * \param any_hit Any Hit Shader Name
		 * \param intersection Intersection Shader Name
		 */
		void AddHitGroup(const std::wstring& hit_group_name, const std::wstring& closest_hit,
		                 const std::wstring& any_hit = L"", const std::wstring& intersection = L"");

		/**
		 * \brief ローカルルートシグネチャと各シンボルを紐付ける
		 * Ray Generation or Miss Shaderの場合はそのままの関数名
		 * Hitの場合はHitGroupの名前
		 * \param root_signature Local Root Signature
		 * \param symbols Symbol Name
		 */
		void AddRootSignatureAssociation(ID3D12RootSignature* root_signature, 
		                                 const std::vector<std::wstring>& symbols);

		/**
		 * \brief RayPayloadの最大サイズ
		 * \param size Size
		 */
		void SetMaxPayloadSize(UINT size) { max_payload_size_ = size; }

		/**
		 * \brief Intersection ShaderからほかHit Shaderへ送る構造体の最大サイズ
		 * \param size Size
		 */
		void SetMaxAttributeSize(UINT size) { max_attribute_size_ = size; }

		/**
		 * \brief 最大再帰数 反射回数の設定（厳密には違うけど）
		 * \param depth Depth
		 */
		void SetMaxRecursionDepth(UINT depth) { max_recursion_depth_ = depth; }

		/**
		 * \brief パイプラインの作成
		 * Localは多分ダミーで適当なやつを用意してもいい
		 * Globalはシーンコンスタントとか背景とかバインドするもんがあるので
		 * ちゃんと作ったやつをバインドするのが望ましいと思う
		 * \param graphics_device GraphicsDevice
		 * \param global_root_signature Global Root Signature
		 * \param local_root_signature Local Root Signature
		 */
		void Generate(const GraphicsDevice* graphics_device,
		              const ID3D12RootSignature* global_root_signature,
		              const ID3D12RootSignature* local_root_signature);

		/**
		 * \brief ID3D12StateObjectを取得
		 * \return ID3D12StateObject
		 */
		ID3D12StateObject* GetStateObject() const { return state_object_.Get(); }

		/**
		 * \brief ID3D12StateObjectPropertiesを取得
		 * \return ID3D12StateObjectProperties
		 */
		ID3D12StateObjectProperties* GetStateObjectProperties() const { return state_object_properties_.Get(); }

	private:
		/**
		 * \brief Shader Library記録用
		 */
		struct ShaderLibrary
		{
			ShaderLibrary(IDxcBlob* p_dxc_blob, const std::vector<std::wstring>& export_symbol);
			IDxcBlob* p_dxc_blob_{};	
			std::vector<std::wstring> export_symbols_{};
			std::vector<D3D12_EXPORT_DESC> export_desc_{};
			D3D12_DXIL_LIBRARY_DESC library_desc_{};
		};

		/**
		 * \brief Hitグループ記憶用
		 */
		struct HitGroup
		{
			HitGroup(std::wstring hit_group_name, std::wstring closest_hit,
				std::wstring any_hit = L"", std::wstring intersection = L"");

			std::wstring hit_group_name_{};
			std::wstring closest_hit_symbol_{};
			std::wstring any_hit_symbol_{};
			std::wstring intersection_symbol_{};
			D3D12_HIT_GROUP_DESC hit_group_desc_{};
		};

		/**
		 * \brief ルートシグネチャとシンボルの関係保持用
		 */
		struct RootSignatureAssociation
		{
			RootSignatureAssociation(ID3D12RootSignature* root_signature, 
				const std::vector<std::wstring>& symbols);

			ID3D12RootSignature* root_signature_{};
			std::vector<std::wstring> symbols_{};
			std::vector<LPCWSTR> symbol_pointers_{};
			D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION association_{};
		};

	private:

		//Object
		Microsoft::WRL::ComPtr<ID3D12StateObject> state_object_{};
		Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> state_object_properties_{};

		//データ保持用
		std::vector<ShaderLibrary> shader_libraries_{};
		std::vector<HitGroup> hit_groups_{};
		std::vector<RootSignatureAssociation> root_signature_associations_{};
		UINT max_payload_size_{};
		UINT max_attribute_size_{};
		UINT max_recursion_depth_{};
	};
}