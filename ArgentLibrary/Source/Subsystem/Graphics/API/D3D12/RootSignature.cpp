#include "Subsystem/Graphics/API/D3D12/RootSignature.h"

#include "Subsystem/Graphics/API/D3D12/GraphicsDevice.h"


namespace argent::graphics::dx12
{
	void RootSignature::AddHeapRangeParameter(UINT base_shader_register, UINT num_descriptors,
		D3D12_DESCRIPTOR_RANGE_TYPE range_type, UINT register_space, UINT offset_in_descriptors_from_table_start)
	{
		std::vector<D3D12_DESCRIPTOR_RANGE> range(1);
		range.at(0).NumDescriptors = num_descriptors;
		range.at(0).BaseShaderRegister = base_shader_register;
		range.at(0).RangeType = range_type;
		range.at(0).RegisterSpace = register_space;
		range.at(0).OffsetInDescriptorsFromTableStart = offset_in_descriptors_from_table_start;
		AddHeapRangeParameters(range);
	}

	void RootSignature::AddHeapRangeParameters(
		std::vector<std::tuple<UINT, UINT, UINT, D3D12_DESCRIPTOR_RANGE_TYPE>> ranges)
	{
		std::vector<D3D12_DESCRIPTOR_RANGE> range_storage;
		range_storage.reserve(ranges.size());
		for(const auto& range : ranges)
		{
			D3D12_DESCRIPTOR_RANGE r{};
			r.BaseShaderRegister = std::get<BaseShaderRegister>(range);
			r.NumDescriptors = std::get<NumDescriptors>(range);
			r.RegisterSpace = std::get<RegisterSpace>(range);
			r.RangeType = std::get<RangeType>(range);
			r.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			range_storage.push_back(r);
		}
		AddHeapRangeParameters(range_storage);
	}

	void RootSignature::AddHeapRangeParameters(const std::vector<D3D12_DESCRIPTOR_RANGE>& ranges)
	{
		descriptor_ranges_.push_back(ranges);

		D3D12_ROOT_PARAMETER param{};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(ranges.size());
		param.DescriptorTable.pDescriptorRanges = nullptr;

		root_parameters_.push_back(param);
		range_locations_.push_back(static_cast<UINT>(descriptor_ranges_.size() - 1));
	}

	void RootSignature::AddRootParameter(D3D12_ROOT_PARAMETER_TYPE type, UINT shader_register, UINT register_space,
		UINT num_root_constants)
	{
		D3D12_ROOT_PARAMETER param{};
		param.ParameterType = type;

		if(type == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
		{
			param.Constants.Num32BitValues = num_root_constants;
			param.Constants.RegisterSpace = register_space;
			param.Constants.ShaderRegister = shader_register;
		}
		else
		{
			param.Descriptor.RegisterSpace = register_space;
			param.Descriptor.ShaderRegister = shader_register;
		}

		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		root_parameters_.push_back(param);
		range_locations_.push_back(~0);
	}

	void RootSignature::Create(const GraphicsDevice* graphics_device, bool is_local)
	{

		for(size_t i = 0; i < root_parameters_.size(); ++i)
		{
			if(root_parameters_.at(i).ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			{
				root_parameters_.at(i).DescriptorTable.pDescriptorRanges = 
					descriptor_ranges_.at(range_locations_.at(i)).data();
			}
		}

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.NumParameters = static_cast<UINT>(root_parameters_.size());
		desc.pParameters = root_parameters_.data();
		desc.Flags = is_local ? D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE :
			D3D12_ROOT_SIGNATURE_FLAG_NONE;

		graphics_device->SerializeAndCreateRootSignature(desc,
			root_signature_object_.ReleaseAndGetAddressOf());
	}
}
