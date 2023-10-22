#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <memory>
#include <unordered_map>

#include "BottomLevelAccelerationStructure.h"
#include "TopLevelAccelerationStructure.h"

namespace argent::graphics::dxr
{
	class AccelerationStructureManager
	{
	public:
		AccelerationStructureManager() = default;


		UINT AddBottomLevelAS(const GraphicsDevice* graphics_device, 
			const GraphicsCommandList* graphics_command_list, 
			BLASBuildDesc* build_desc, bool is_triangle);
		UINT RegisterTopLevelAS(UINT blas_unique_id, 
			UINT hit_group_index, const DirectX::XMFLOAT4X4& world, bool front_counter_clockwise);

		BottomLevelAccelerationStructure* GetBottomLevelAS(UINT unique_id) const;

		void Update(const GraphicsCommandList* graphics_command_list);
		void Generate(const GraphicsDevice* graphics_device, 
		const GraphicsCommandList* graphics_command_list);
		UINT GetInstanceCounts() const { return static_cast<UINT>(tlas_un_map_.size()); }

		ID3D12Resource* GetResultResourceObject() const { return result_buffer_object_.Get(); }

		void SetWorld(const DirectX::XMFLOAT4X4& world, UINT tlas_unique_id);
	private:
		static UINT GenerateUniqueID();
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> tlas_transform_;
		Microsoft::WRL::ComPtr<ID3D12Resource> blas_transform_;

		std::unordered_map<UINT, std::unique_ptr<TopLevelAccelerationStructure>> tlas_un_map_;
		std::vector<std::unique_ptr<BottomLevelAccelerationStructure>> blas_vec_;

		Microsoft::WRL::ComPtr<ID3D12Resource> scratch_buffer_object_;
		Microsoft::WRL::ComPtr<ID3D12Resource> result_buffer_object_;
		Microsoft::WRL::ComPtr<ID3D12Resource> instance_desc_buffer_object_;
		UINT scratch_buffer_size_;
		UINT result_buffer_size_;
		UINT instance_desc_buffer_size_;
	};
}