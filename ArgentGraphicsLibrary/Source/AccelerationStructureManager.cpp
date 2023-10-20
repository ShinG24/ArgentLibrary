#include "../Inc/AccelerationStructureManager.h"

#include "../Inc/GraphicsDevice.h"
#include "../Inc/GraphicsCommandList.h"
#include "../Inc/GraphicsCommon.h"

namespace argent::graphics::dxr
{
	UINT AccelerationStructureManager::AddBottomLevelAS(const GraphicsDevice* graphics_device, 
		const GraphicsCommandList* graphics_command_list, BLASBuildDesc* build_desc, bool is_triangle)
	{
		UINT unique_id = GenerateUniqueID();
		blas_vec_.emplace_back(std::make_unique<BottomLevelAccelerationStructure>(graphics_device, graphics_command_list, 
			build_desc, unique_id, is_triangle));
		return unique_id;
	}

	UINT AccelerationStructureManager::RegisterTopLevelAS(UINT blas_unique_id, 
		UINT hit_group_index, const DirectX::XMFLOAT4X4& world)
	{
		UINT unique_id = GenerateUniqueID();

		tlas_un_map_.emplace(unique_id, std::make_unique<TopLevelAccelerationStructure>(unique_id, 
			blas_unique_id, GetBottomLevelAS(blas_unique_id)->GetResultBuffer()->GetGPUVirtualAddress(),
			hit_group_index, world));
		return unique_id;
	}

	BottomLevelAccelerationStructure* AccelerationStructureManager::GetBottomLevelAS(UINT unique_id) const 
	{
		for(const auto& b : blas_vec_)
		{
			if(b->GetUniqueID() == unique_id)
			{
				return b.get();
			}
		}
		return nullptr;
	}

	void AccelerationStructureManager::Update(const GraphicsCommandList* graphics_command_list)
	{
		D3D12_RAYTRACING_INSTANCE_DESC* instance_desc;
		HRESULT hr = instance_desc_buffer_object_->Map(0u, nullptr, reinterpret_cast<void**>(&instance_desc));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Map");

		const UINT instance_counts = static_cast<UINT>(tlas_un_map_.size());

		UINT i = 0; 
		for(const auto& t : tlas_un_map_)
		{
			instance_desc[i] = t.second->GetD3D12InstanceDesc();
			++i;
		}

		instance_desc_buffer_object_->Unmap(0u, nullptr);

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc{};
		build_desc.DestAccelerationStructureData = result_buffer_object_->GetGPUVirtualAddress();
		build_desc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		build_desc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
		build_desc.Inputs.NumDescs = instance_counts;
		build_desc.Inputs.InstanceDescs = instance_desc_buffer_object_->GetGPUVirtualAddress();
		build_desc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		build_desc.ScratchAccelerationStructureData = scratch_buffer_object_->GetGPUVirtualAddress();
		build_desc.SourceAccelerationStructureData = result_buffer_object_->GetGPUVirtualAddress();
		
		graphics_command_list->GetCommandList4()->BuildRaytracingAccelerationStructure(&build_desc, 0u, nullptr);
		graphics_command_list->SetUavBarrier(result_buffer_object_.Get(), D3D12_RESOURCE_BARRIER_FLAG_NONE);
	}

	void AccelerationStructureManager::Generate(const GraphicsDevice* graphics_device, 
	                                            const GraphicsCommandList* graphics_command_list)
	{
		//Compute Buffer size
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS as_input{};
		as_input.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		as_input.NumDescs = static_cast<UINT>(tlas_un_map_.size());
		as_input.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		as_input.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO as_info;
		graphics_device->GetLatestDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&as_input, &as_info);

		scratch_buffer_size_ = _ALIGNMENT_(as_info.ScratchDataSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		result_buffer_size_ = _ALIGNMENT_(as_info.ResultDataMaxSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		instance_desc_buffer_size_ = _ALIGNMENT_(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * tlas_un_map_.size(), 
			D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		graphics_device->CreateBuffer(kDefaultHeapProp, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, 
			scratch_buffer_size_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, scratch_buffer_object_.ReleaseAndGetAddressOf());

		graphics_device->CreateBuffer(kDefaultHeapProp, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			result_buffer_size_, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, result_buffer_object_.ReleaseAndGetAddressOf());

		graphics_device->CreateBuffer(kUploadHeapProp, D3D12_RESOURCE_FLAG_NONE, 
			instance_desc_buffer_size_, D3D12_RESOURCE_STATE_GENERIC_READ, instance_desc_buffer_object_.ReleaseAndGetAddressOf());


		D3D12_RAYTRACING_INSTANCE_DESC* instance_desc;
		HRESULT hr = instance_desc_buffer_object_->Map(0u, nullptr, reinterpret_cast<void**>(&instance_desc));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Map");

		const UINT instance_counts = static_cast<UINT>(tlas_un_map_.size());

		SecureZeroMemory(instance_desc, instance_desc_buffer_size_);

		UINT i = 0; 
		for(const auto& t : tlas_un_map_)
		{
			instance_desc[i] = t.second->GetD3D12InstanceDesc();
			++i;
		}

		instance_desc_buffer_object_->Unmap(0u, nullptr);

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc{};
		build_desc.DestAccelerationStructureData = result_buffer_object_->GetGPUVirtualAddress();
		build_desc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		build_desc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
		build_desc.Inputs.NumDescs = instance_counts;
		build_desc.Inputs.InstanceDescs = instance_desc_buffer_object_->GetGPUVirtualAddress();
		build_desc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		build_desc.ScratchAccelerationStructureData = scratch_buffer_object_->GetGPUVirtualAddress();
		build_desc.SourceAccelerationStructureData = 0;
		
		graphics_command_list->GetCommandList4()->BuildRaytracingAccelerationStructure(&build_desc, 0u, nullptr);
		graphics_command_list->SetUavBarrier(result_buffer_object_.Get(), D3D12_RESOURCE_BARRIER_FLAG_NONE);
	}

	void AccelerationStructureManager::SetWorld(const DirectX::XMFLOAT4X4& world, UINT tlas_unique_id)
	{
		tlas_un_map_[tlas_unique_id]->SetWorld(world);
	}

	UINT AccelerationStructureManager::GenerateUniqueID()
	{
		static UINT unique_id = 0;
		return unique_id++;
	}
}
