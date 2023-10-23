#include "../Inc/RaytracingPipelineState.h"

#include <unordered_set>
#include <vector>

#include "../Inc/GraphicsDevice.h"


namespace argent::graphics::dxr
{
	void RaytracingPipelineState::AddLibrary(IDxcBlob* p_dxc_blob, const std::vector<std::wstring>& export_symbol)
	{
		shader_libraries_.emplace_back(
			ShaderLibrary(
			p_dxc_blob, export_symbol));
	}

	void RaytracingPipelineState::AddHitGroup(const std::wstring& hit_group_name, const std::wstring& closest_hit,
		const std::wstring& any_hit, const std::wstring& intersection)
	{
		hit_groups_.emplace_back(HitGroup(hit_group_name, 
			closest_hit, any_hit, intersection));
	}

	void RaytracingPipelineState::AddRootSignatureAssociation(ID3D12RootSignature* root_signature,
		const std::vector<std::wstring>& symbols)
	{
		root_signature_associations_.emplace_back(
			RootSignatureAssociation(
			root_signature, symbols));
	}

	void RaytracingPipelineState::Generate(const GraphicsDevice* graphics_device,
	                                       const ID3D12RootSignature* global_root_signature,
	                                       const ID3D12RootSignature* local_root_signature)
	{
		UINT current_index = 0u;
		const UINT subobject_counts = 
			static_cast<UINT>(shader_libraries_.size()) + 
			static_cast<UINT>(hit_groups_.size()) + 1 + 1 + 
			2 * static_cast<UINT>(root_signature_associations_.size()) + 
			2 + 1;

		std::vector<D3D12_STATE_SUBOBJECT> subobjects(subobject_counts);

		//For Global And Local Root Signature
		D3D12_STATE_SUBOBJECT grs_sub;
		grs_sub.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
		grs_sub.pDesc = &global_root_signature;
		subobjects.at(current_index) = grs_sub;
		current_index++;

		D3D12_STATE_SUBOBJECT lrs_sub;
		lrs_sub.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
		lrs_sub.pDesc = &local_root_signature;
		subobjects.at(current_index) = lrs_sub;
		current_index++;

		//For Shader Libraries
		for(const auto& lib : shader_libraries_)
		{
			D3D12_STATE_SUBOBJECT library_sub{};
			library_sub.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
			library_sub.pDesc = &lib.library_desc_;
			subobjects[current_index] = library_sub;
			current_index++;
		}

		//For Hit Group
		for(auto& hit : hit_groups_)
		{
			D3D12_STATE_SUBOBJECT hit_sub{};
			hit_sub.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;

			//Sometime crash here
			//The hitgroupdesc.closesthitshaderimport are missing.
			//but hit.closesthitsymbol has valid value.
			hit.hit_group_desc_.ClosestHitShaderImport = hit.closest_hit_symbol_.empty() ?
				nullptr : hit.closest_hit_symbol_.c_str();
			hit.hit_group_desc_.AnyHitShaderImport = hit.any_hit_symbol_.empty() ?
				nullptr : hit.any_hit_symbol_.c_str();

			hit.hit_group_desc_.IntersectionShaderImport = hit.intersection_symbol_.empty() ?
				nullptr : hit.intersection_symbol_.c_str();

			hit_sub.pDesc = &hit.hit_group_desc_;
			subobjects[current_index] = hit_sub;
			current_index++;
		}

		//For Shader Config
		D3D12_RAYTRACING_SHADER_CONFIG shader_config{};
		shader_config.MaxAttributeSizeInBytes = max_attribute_size_;
		shader_config.MaxPayloadSizeInBytes = max_payload_size_;
		D3D12_STATE_SUBOBJECT shader_config_sub{};
		shader_config_sub.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		shader_config_sub.pDesc = &shader_config;
		subobjects[current_index] = shader_config_sub;
		current_index++;

		//For Shader Export Association
		std::vector<std::wstring> exported_symbols{};
		std::vector<LPCWSTR> exported_symbol_pointers{};
		//Build Exported Symbols
		{
			std::unordered_set<std::wstring> exports;
			//Accumulate Library export Symbols
			for(const auto& lib :shader_libraries_)
			{
				for(const auto& name : lib.export_symbols_)
				{
					exports.insert(name);
				}
			}

			//Accumulate HitGroup name
			for(const auto& hit : hit_groups_)
			{
				//We dont need closets, any and intersection shader symbol
				//because these are tied to hit_group_name;
				if(!hit.any_hit_symbol_.empty()) exports.erase(hit.any_hit_symbol_);
				if(!hit.closest_hit_symbol_.empty()) exports.erase(hit.closest_hit_symbol_);
				if(!hit.intersection_symbol_.empty()) exports.erase(hit.intersection_symbol_);
				exports.insert(hit.hit_group_name_);
			}

			for(const auto& name : exports)
			{
				exported_symbols.emplace_back(name);
			}
		}
		exported_symbol_pointers.reserve(exported_symbols.size());
		for(const auto& name : exported_symbols)
		{
			exported_symbol_pointers.push_back(name.c_str());
		}
		const WCHAR** shader_exports = exported_symbol_pointers.data();
		D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shader_payload_association{};
		shader_payload_association.NumExports = static_cast<UINT>(exported_symbols.size());
		shader_payload_association.pExports = shader_exports;
		shader_payload_association.pSubobjectToAssociate = &subobjects[current_index - 1];
		D3D12_STATE_SUBOBJECT export_ass_sub{};
		export_ass_sub.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
		export_ass_sub.pDesc = &shader_payload_association;
		subobjects[current_index] = export_ass_sub;
		current_index++;

		//For Rootsignature Association
		for(auto& ass : root_signature_associations_)
		{
			D3D12_STATE_SUBOBJECT root_sig_sub{};
			root_sig_sub.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
			root_sig_sub.pDesc = &ass.root_signature_;
			subobjects[current_index] = root_sig_sub;
			current_index++;

			ass.association_.NumExports = static_cast<UINT>(ass.symbol_pointers_.size());
			ass.association_.pExports = ass.symbol_pointers_.data();
			ass.association_.pSubobjectToAssociate = &subobjects[current_index - 1];
			D3D12_STATE_SUBOBJECT root_sig_ass_sub{};
			root_sig_ass_sub.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
			root_sig_ass_sub.pDesc = &ass.association_;
			subobjects[current_index] = root_sig_ass_sub;
			current_index++;
		}

		//For Pipeline Config
		D3D12_RAYTRACING_PIPELINE_CONFIG pipeline_config{};
		pipeline_config.MaxTraceRecursionDepth = max_recursion_depth_;
		D3D12_STATE_SUBOBJECT pipeline_config_sub{};
		pipeline_config_sub.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
		pipeline_config_sub.pDesc = &pipeline_config;

		subobjects[current_index] = pipeline_config_sub;
		current_index++;

		if(current_index != subobjects.size()) _ASSERT_EXPR(FALSE, L"Miss");

		//Final Generation
		{
			D3D12_STATE_OBJECT_DESC desc{};
			desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
			desc.NumSubobjects = subobject_counts;
			desc.pSubobjects = subobjects.data();

			HRESULT hr = graphics_device->GetLatestDevice()->CreateStateObject(&desc, 
				IID_PPV_ARGS(state_object_.ReleaseAndGetAddressOf()));
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Raytarcing State Object");

			hr = state_object_->QueryInterface(IID_PPV_ARGS(state_object_properties_.ReleaseAndGetAddressOf()));
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Query Interface");
		}
	}

	RaytracingPipelineState::ShaderLibrary::ShaderLibrary(IDxcBlob* p_dxc_blob,
		const std::vector<std::wstring>& export_symbol) :
		p_dxc_blob_(p_dxc_blob)
	,	export_symbols_(export_symbol)
	,	export_desc_(export_symbols_.size())
	{
		for(size_t i = 0; i < export_symbols_.size(); ++i)
		{
			export_desc_.at(i).Name = export_symbols_.at(i).c_str();
			export_desc_.at(i).ExportToRename = nullptr;
			export_desc_.at(i).Flags = D3D12_EXPORT_FLAG_NONE;
		}

		library_desc_.DXILLibrary.pShaderBytecode = p_dxc_blob_->GetBufferPointer();
		library_desc_.DXILLibrary.BytecodeLength = p_dxc_blob_->GetBufferSize();
		library_desc_.NumExports = static_cast<UINT>(export_symbols_.size());
		library_desc_.pExports = export_desc_.data();
	}

	RaytracingPipelineState::HitGroup::HitGroup(std::wstring hit_group_name, std::wstring closest_hit,
		std::wstring any_hit, std::wstring intersection):
		hit_group_name_(hit_group_name)
	,	closest_hit_symbol_(closest_hit)
	,	any_hit_symbol_(any_hit)
	,	intersection_symbol_(intersection)
	{
		hit_group_desc_.HitGroupExport = hit_group_name_.c_str();
		hit_group_desc_.ClosestHitShaderImport = closest_hit_symbol_.empty() ? 
				nullptr : closest_hit_symbol_.c_str();
		hit_group_desc_.AnyHitShaderImport = any_hit_symbol_.empty() ? 
				nullptr : any_hit_symbol_.c_str();
		hit_group_desc_.IntersectionShaderImport = intersection_symbol_.empty() ? 
				nullptr : intersection_symbol_.c_str();
		hit_group_desc_.Type = intersection_symbol_.empty() ?
			D3D12_HIT_GROUP_TYPE_TRIANGLES : D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;
	}

	RaytracingPipelineState::RootSignatureAssociation::RootSignatureAssociation(ID3D12RootSignature* root_signature,
		const std::vector<std::wstring>& symbols):
		root_signature_(root_signature)
	,	symbols_(symbols)
	,	symbol_pointers_(symbols.size())
	{
		for(size_t i = 0; i < symbols_.size(); ++i)
		{
			symbol_pointers_.at(i) = symbols_.at(i).c_str();
		}
	}
}
