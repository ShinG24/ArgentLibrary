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

	class RaytracingPipelineState
	{
	public:
		RaytracingPipelineState() = default;
		~RaytracingPipelineState() = default;

		RaytracingPipelineState(const RaytracingPipelineState&) = delete;
		RaytracingPipelineState(const RaytracingPipelineState&&) = delete;
		RaytracingPipelineState& operator=(const RaytracingPipelineState&) = delete;
		RaytracingPipelineState& operator=(const RaytracingPipelineState&&) = delete;


		void AddLibrary(IDxcBlob* p_dxc_blob, const std::vector<std::wstring>& export_symbol);
		void AddHitGroup(const std::wstring& hit_group_name, const std::wstring& closest_hit,
			const std::wstring& any_hit = L"", const std::wstring& intersection = L"");

		void AddRootSignatureAssociation(ID3D12RootSignature* root_signature, 
			const std::vector<std::wstring>& symbols);

		void SetMaxPayloadSize(UINT size) { max_payload_size_ = size; }
		void SetMaxAttributeSize(UINT size) { max_attribute_size_ = size; }
		void SetMaxRecursionDepth(UINT depth) { max_recursion_depth_ = depth; }

		void Generate(const GraphicsDevice* graphics_device,
			const ID3D12RootSignature* global_root_signature,
			const ID3D12RootSignature* local_root_signature);


		ID3D12StateObject* GetStateObject() const { return state_object_.Get(); }
		ID3D12StateObjectProperties* GetStateObjectProperties() const { return state_object_properties_.Get(); }

	private:

		struct ShaderLibrary
		{
			ShaderLibrary(IDxcBlob* p_dxc_blob, const std::vector<std::wstring>& export_symbol);
			IDxcBlob* p_dxc_blob_{};
			std::vector<std::wstring> export_symbols_{};
			std::vector<D3D12_EXPORT_DESC> export_desc_{};
			D3D12_DXIL_LIBRARY_DESC library_desc_{};
		};

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

		Microsoft::WRL::ComPtr<ID3D12StateObject> state_object_;
		Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> state_object_properties_;

		std::vector<ShaderLibrary> shader_libraries_;
		std::vector<HitGroup> hit_groups_;
		std::vector<RootSignatureAssociation> root_signature_associations_;
		UINT max_payload_size_;
		UINT max_attribute_size_;
		UINT max_recursion_depth_;
	};
}