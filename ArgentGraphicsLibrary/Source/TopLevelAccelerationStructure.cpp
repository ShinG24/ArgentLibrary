#include "../Inc/TopLevelAccelerationStructure.h"

#include "../Inc/BottomLevelAccelerationStructure.h"
#include "../Inc/GraphicsCommon.h"
#include "../Inc/GraphicsDevice.h"
#include "../Inc/GraphicsCommandList.h"

namespace argent::graphics
{
	void TopLevelAccelerationStructure::AddInstance(BottomLevelAccelerationStructure* blas,
		const DirectX::XMMATRIX& m, UINT hit_group_index)
	{
		UINT blas_index = 0xffff;
		for(UINT i = 0; static_cast<UINT>(i < blas_.size()); ++i)
		{
			if(blas_.at(i) == blas) { blas_index = i; }
		}

		if(blas_index == 0xffff)
		{
			blas_.emplace_back(blas);
			blas_index = static_cast<UINT>(blas_.size()) - 1;
		}

		Instance instance{};
		instance.blas_index_ = blas_index;
		instance.m_ = m;
		instance.instance_index_ = static_cast<UINT>(instances_.size());
		instance.hit_group_index_ = hit_group_index;
		instances_.emplace_back(instance);
	}

	void TopLevelAccelerationStructure::Generate(const GraphicsDevice* graphics_device,
		const GraphicsCommandList* graphics_command_list)
	{
		//Compute Buffer size
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS as_input{};
		as_input.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		as_input.NumDescs = static_cast<UINT>(instances_.size());
		as_input.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		as_input.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO as_info;
		graphics_device->GetLatestDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&as_input, &as_info);

		scratch_buffer_size_ = _ALIGNMENT_(as_info.ScratchDataSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		result_buffer_size_ = _ALIGNMENT_(as_info.ResultDataMaxSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		instance_buffer_size_ = _ALIGNMENT_(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instances_.size(), 
			D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		graphics_device->CreateBuffer(kDefaultHeapProp, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, 
			scratch_buffer_size_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, scratch_buffer_object_.ReleaseAndGetAddressOf());

		graphics_device->CreateBuffer(kDefaultHeapProp, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			result_buffer_size_, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, result_buffer_object_.ReleaseAndGetAddressOf());

		graphics_device->CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, 
			instance_buffer_size_, D3D12_RESOURCE_STATE_GENERIC_READ, instance_buffer_object_.ReleaseAndGetAddressOf());


		D3D12_RAYTRACING_INSTANCE_DESC* instance_desc;
		HRESULT hr = instance_buffer_object_->Map(0u, nullptr, reinterpret_cast<void**>(&instance_desc));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Map");

		const UINT instance_counts = static_cast<UINT>(instances_.size());

		SecureZeroMemory(instance_desc, instance_buffer_size_);

		for(UINT i = 0; i < instance_counts; ++i)
		{
			const auto& instance = instances_.at(i);
			instance_desc[i].InstanceID = instance.instance_index_;
			instance_desc[i].InstanceContributionToHitGroupIndex = instance.hit_group_index_;
			instance_desc[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
			DirectX::XMMATRIX m = DirectX::XMMatrixTranspose(instance.m_);
			memcpy(instance_desc[i].Transform, &m, sizeof(instance_desc[i].Transform));
			instance_desc[i].AccelerationStructure = blas_.at(instance.blas_index_)->GetResultBuffer()->GetGPUVirtualAddress();
			instance_desc[i].InstanceMask = 0xff;
		}

		instance_buffer_object_->Unmap(0u, nullptr);

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc{};
		build_desc.DestAccelerationStructureData = result_buffer_object_->GetGPUVirtualAddress();
		build_desc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		build_desc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
		build_desc.Inputs.NumDescs = instance_counts;
		build_desc.Inputs.InstanceDescs = instance_buffer_object_->GetGPUVirtualAddress();
		build_desc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		build_desc.ScratchAccelerationStructureData = scratch_buffer_object_->GetGPUVirtualAddress();
		build_desc.SourceAccelerationStructureData = 0;
		
		graphics_command_list->GetCommandList4()->BuildRaytracingAccelerationStructure(&build_desc, 0u, nullptr);
		graphics_command_list->SetUavBarrier(result_buffer_object_.Get(), D3D12_RESOURCE_BARRIER_FLAG_NONE);
	}

	void TopLevelAccelerationStructure::Update(const GraphicsCommandList* graphics_command_list)
	{
		D3D12_RAYTRACING_INSTANCE_DESC* instance_desc;
		HRESULT hr = instance_buffer_object_->Map(0u, nullptr, reinterpret_cast<void**>(&instance_desc));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Map");

		const UINT instance_counts = static_cast<UINT>(instances_.size());

		for(UINT i = 0; i < instance_counts; ++i)
		{
			const auto& instance = instances_.at(i);
			instance_desc[i].InstanceID = instance.instance_index_;
			instance_desc[i].InstanceContributionToHitGroupIndex = instance.hit_group_index_;
			instance_desc[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
			DirectX::XMMATRIX m = DirectX::XMMatrixTranspose(instance.m_);
			memcpy(instance_desc[i].Transform, &m, sizeof(instance_desc[i].Transform));
			instance_desc[i].AccelerationStructure = blas_.at(instance.blas_index_)->GetResultBuffer()->GetGPUVirtualAddress();
			instance_desc[i].InstanceMask = 0xff;
		}

		instance_buffer_object_->Unmap(0u, nullptr);

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc{};
		build_desc.DestAccelerationStructureData = result_buffer_object_->GetGPUVirtualAddress();
		build_desc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		build_desc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
		build_desc.Inputs.NumDescs = instance_counts;
		build_desc.Inputs.InstanceDescs = instance_buffer_object_->GetGPUVirtualAddress();
		build_desc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		build_desc.ScratchAccelerationStructureData = scratch_buffer_object_->GetGPUVirtualAddress();
		build_desc.SourceAccelerationStructureData = result_buffer_object_->GetGPUVirtualAddress();
		
		graphics_command_list->GetCommandList4()->BuildRaytracingAccelerationStructure(&build_desc, 0u, nullptr);
		graphics_command_list->SetUavBarrier(result_buffer_object_.Get(), D3D12_RESOURCE_BARRIER_FLAG_NONE);

	}
}
