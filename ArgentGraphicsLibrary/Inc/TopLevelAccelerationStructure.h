#pragma once

#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>

#include <vector>


namespace argent::graphics
{
	class GraphicsDevice;
	class GraphicsCommandList;

	class BottomLevelAccelerationStructure;
	class TopLevelAccelerationStructure
	{
	public:
		TopLevelAccelerationStructure() = default;
		~TopLevelAccelerationStructure() = default;

		void AddInstance(BottomLevelAccelerationStructure* blas,
		const DirectX::XMMATRIX& m, UINT hit_group_index);

		void Generate(const GraphicsDevice* graphics_device,
		const GraphicsCommandList* graphics_command_list);

		void Update();

		ID3D12Resource* GetResultBuffer() const { return result_buffer_object_.Get(); }

	private:
		struct Instance
		{
			UINT instance_index_;
			UINT blas_index_;
			UINT hit_group_index_;
			DirectX::XMMATRIX m_;
		};

		std::vector<BottomLevelAccelerationStructure*> blas_;
		std::vector<Instance> instances_;

			Microsoft::WRL::ComPtr<ID3D12Resource> scratch_buffer_object_;
		Microsoft::WRL::ComPtr<ID3D12Resource> result_buffer_object_;
		Microsoft::WRL::ComPtr<ID3D12Resource> instance_buffer_object_;

		UINT scratch_buffer_size_;
		UINT result_buffer_size_;
		UINT instance_buffer_size_;
	};
}
