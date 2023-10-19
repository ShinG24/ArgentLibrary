#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <memory>

#include "BottomLevelAccelerationStructure.h"
#include "TopLevelAccelerationStructure.h"

namespace argent::graphics::dxr
{
	class AccelerationStructure
	{
	public:
		AccelerationStructure();


	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> tlas_transform_;
		Microsoft::WRL::ComPtr<ID3D12Resource> blas_transform_;

		std::vector<std::unique_ptr<BottomLevelAccelerationStructure>> blas_vec_;
		TopLevelAccelerationStructure tlas_;
	};
}