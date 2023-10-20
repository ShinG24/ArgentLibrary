#include "../Inc/TopLevelAccelerationStructure.h"

#include "../Inc/BottomLevelAccelerationStructure.h"
#include "../Inc/GraphicsCommon.h"
#include "../Inc/GraphicsDevice.h"
#include "../Inc/GraphicsCommandList.h"

namespace argent::graphics::dxr
{
	TopLevelAccelerationStructure::TopLevelAccelerationStructure(UINT unique_id, UINT blas_unique_id, D3D12_GPU_VIRTUAL_ADDRESS blas_gpu_address,
		UINT hit_group_index, const DirectX::XMFLOAT4X4& world_matrix):
		unique_id_(unique_id)
	,	blas_unique_id_(blas_unique_id)
	,	hit_group_index_(hit_group_index)
	,	world_matrix_(world_matrix)
	{
		instance_desc_.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
		instance_desc_.AccelerationStructure = blas_gpu_address;
		instance_desc_.InstanceContributionToHitGroupIndex = hit_group_index_;
		instance_desc_.InstanceID = unique_id_;
		instance_desc_.InstanceMask = 0xFF;
		DirectX::XMMATRIX m = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&world_matrix_));
		memcpy(&instance_desc_.Transform, &m, sizeof(instance_desc_.Transform));
	}

	void TopLevelAccelerationStructure::SetWorld(const DirectX::XMFLOAT4X4& world)
	{
		world_matrix_ = world;
		DirectX::XMMATRIX m = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&world_matrix_));
		memcpy(&instance_desc_.Transform, &m, sizeof(instance_desc_.Transform));
	}
}
