#pragma once

#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>

#include <vector>

namespace argent::graphics
{
	class GraphicsDevice;
	class GraphicsCommandList;
}

namespace argent::graphics::dxr
{
	class BottomLevelAccelerationStructure;

	class TopLevelAccelerationStructure
	{
	public:
		TopLevelAccelerationStructure(UINT unique_id, UINT blas_unique_id, D3D12_GPU_VIRTUAL_ADDRESS blas_gpu_address,
		UINT hit_group_index, const DirectX::XMFLOAT4X4& world_matrix, bool front_counter_clockwise);
		~TopLevelAccelerationStructure() = default;

		D3D12_RAYTRACING_INSTANCE_DESC GetD3D12InstanceDesc() const { return instance_desc_; }
		void SetWorld(const DirectX::XMFLOAT4X4& world);
	private:
		UINT unique_id_;
		UINT blas_unique_id_;
		UINT hit_group_index_;
		DirectX::XMFLOAT4X4 world_matrix_;
		D3D12_RAYTRACING_INSTANCE_DESC instance_desc_;
	};
}
