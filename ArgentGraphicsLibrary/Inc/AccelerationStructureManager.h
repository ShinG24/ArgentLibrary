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
		AccelerationStructureManager();

		UINT AddBottomLevelAS(GraphicsDevice* graphics_device, 
		GraphicsCommandList* graphics_command_list, BLASBuildDesc* build_desc);
		void RegisterTopLevelAS();

		void GetBottomLevelAS(UINT unique_id);

	private:
		static UINT GenerateUniqueID();
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> tlas_transform_;
		Microsoft::WRL::ComPtr<ID3D12Resource> blas_transform_;

		std::vector<std::unique_ptr<BottomLevelAccelerationStructure>> blas_vec_;
		TopLevelAccelerationStructure tlas_;
	};
}