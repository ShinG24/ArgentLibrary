#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <memory>

#include "BottomLevelAccelerationStructure.h"
#include "TopLevelAccelerationStructure.h"

namespace argent::graphics::dxr
{
	class AccelerationStructureManager
	{
	public:
		AccelerationStructureManager() = default;


		UINT AddBottomLevelAS(const GraphicsDevice* graphics_device, 
			const GraphicsCommandList* graphics_command_list, BLASBuildDesc* build_desc);
		void RegisterTopLevelAS(UINT blas_unique_id, 
			UINT hit_group_index, const DirectX::XMFLOAT4X4& world) const;

		BottomLevelAccelerationStructure* GetBottomLevelAS(UINT unique_id) const;

		void Update(const GraphicsCommandList* graphics_command_list);
		void Generate(const GraphicsDevice* graphics_device, 
		const GraphicsCommandList* graphics_command_list);

	private:
		static UINT GenerateUniqueID();
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> tlas_transform_;
		Microsoft::WRL::ComPtr<ID3D12Resource> blas_transform_;

		std::vector<std::unique_ptr<BottomLevelAccelerationStructure>> blas_vec_;
		std::vector<std::unique_ptr<TopLevelAccelerationStructure>> tlas_vec_;

		Microsoft::WRL::ComPtr<ID3D12Resource> scratch_buffer_object_;
		Microsoft::WRL::ComPtr<ID3D12Resource> result_buffer_object_;
		Microsoft::WRL::ComPtr<ID3D12Resource> instance_desc_buffer_object_;
		UINT scratch_buffer_size_;
		UINT result_buffer_size_;
		UINT instance_desc_buffer_size_;
	};
}