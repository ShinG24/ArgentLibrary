#include "../Inc/Fence.h"

#include "../Inc/GraphicsDevice.h"


namespace argent::graphics
{
	void Fence::Awake(const GraphicsDevice& graphics_device)
	{
		fence_values_[0] = 0u;
		fence_values_[1] = 0u;
		fence_values_[2] = 0u;
		graphics_device.CreateFence(fence_object_.ReleaseAndGetAddressOf(), fence_values_[0]);
	}

	UINT64 Fence::GetCompletedValue() const
	{
		return fence_object_->GetCompletedValue();
	}

	void Fence::WaitForGpu(UINT back_buffer_index)
	{
		++current_fence_value_;
		if(GetCompletedValue() < fence_values_[back_buffer_index])
		{
			fence_object_->SetEventOnCompletion(fence_values_[back_buffer_index], event_handle_);
			WaitForSingleObject(event_handle_, INFINITE);
		}
		next_fence_value_ = current_fence_value_ + 1;
		fence_values_[back_buffer_index] = next_fence_value_;
	}
}