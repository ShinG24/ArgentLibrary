#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>

#include <vector>

namespace argent::graphics::dx12
{
	class GraphicsDevice;
	class GraphicsCommandList;

	class VertexBuffer;
	class IndexBuffer;
}

namespace argent::graphics::dx12
{
	/**
	 * \brief �J����
	 * vertex_buffer_vec_.size() need to the same value index_buffer_vec_.size()
	 */
	struct BLASBuildDesc
	{
		std::vector<VertexBuffer*> vertex_buffer_vec_{};
		std::vector<IndexBuffer*> index_buffer_vec_{};
		std::vector<D3D12_GPU_VIRTUAL_ADDRESS> transform_vec_{};
	};

	/**
	 * \brief �J����
	 * DXR�ɂ�����BottomLevelAcceleartionStructure
	 * BuildDesc���g���č\�z�ɕK�v�ȃf�[�^��n��������
	 * �����Buffer���\�z����
	 */
	class BottomLevelAccelerationStructure
	{
	public:
		/**
		 * \brief �R���X�g���N�^
		 * \param graphics_device Graphics Device �T�C�Y�擾�Ɏg�� 
		 * \param graphics_command_list Command List �o�b�t�@�̃r���h�Ɏg��
		 * \param build_desc �r���h�̐ݒ�Ɏg��
		 * \param is_triangle �����AABB�ōs�����ǂ���
		 */
		BottomLevelAccelerationStructure(const GraphicsDevice* graphics_device,
		                                 const GraphicsCommandList* graphics_command_list,
		                                 const BLASBuildDesc* build_desc, bool is_triangle = true);
		BottomLevelAccelerationStructure(const GraphicsDevice* graphics_device, const GraphicsCommandList* graphics_command_list, 
			const BLASBuildDesc* build_desc, UINT unique_id, bool is_triangle = true);

		~BottomLevelAccelerationStructure() = default;

		BottomLevelAccelerationStructure(const BottomLevelAccelerationStructure&) = delete;
		BottomLevelAccelerationStructure(const BottomLevelAccelerationStructure&&) = delete;
		BottomLevelAccelerationStructure& operator=(const BottomLevelAccelerationStructure&&) = delete;
		BottomLevelAccelerationStructure& operator=(const BottomLevelAccelerationStructure&) = delete;

		UINT GetUniqueID() const { return unique_id_; }
		ID3D12Resource* GetResultBuffer() const { return result_buffer_object_.Get(); }

	private:

		UINT unique_id_;
		Microsoft::WRL::ComPtr<ID3D12Resource> scratch_buffer_object_;
		Microsoft::WRL::ComPtr<ID3D12Resource> result_buffer_object_;

		UINT scratch_buffer_size_;
		UINT result_buffer_size_;
	};
}