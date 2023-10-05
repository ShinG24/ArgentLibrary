#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "GraphicsCommon.h"

namespace argent::graphics
{
	class GraphicsDevice;
	class CommandQueue;

	//Wraped class of ID3D12Fence.
	class Fence
	{
	public:
		Fence() = default;
		~Fence() = default;

		Fence(Fence&) = delete;
		Fence(Fence&&) = delete;
		Fence& operator=(Fence&) = delete;
		Fence& operator=(Fence&&) = delete;

		void Awake(const GraphicsDevice& graphics_device);

		void PutUpFence(const CommandQueue& command_queue);
		UINT64 GetFenceValue() const { return current_fence_value_; }
		UINT64 GetCompletedValue() const;
		void WaitForGpu(UINT back_buffer_index);
		void WaitForGpuInCurrentFrame() const;

		ID3D12Fence* GetFence() const { return fence_object_.Get(); }
	private:
		Microsoft::WRL::ComPtr<ID3D12Fence> fence_object_;
		UINT64 fence_values_[kNumBackBuffers];
		HANDLE event_handle_;
		UINT64 current_fence_value_;
	};
}
