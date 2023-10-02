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
		if(GetCompletedValue() < fence_values_[back_buffer_index])
		{
			fence_object_->SetEventOnCompletion(fence_values_[back_buffer_index], event_handle_);
			WaitForSingleObject(event_handle_, INFINITE);
		}
		fence_values_[back_buffer_index] = ++current_fence_value_;
	}
}


    //// Schedule a Signal command in the queue.
    //const UINT64 currentFenceValue = m_fenceValues[m_backBufferIndex];
    //ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

    //// Update the back buffer index.
    //m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    //// If the next frame is not ready to be rendered yet, wait until it is ready.
    //if (m_fence->GetCompletedValue() < m_fenceValues[m_backBufferIndex])
    //{
    //    ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_backBufferIndex], m_fenceEvent.Get()));
    //    WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);
    //}

    //// Set the fence value for the next frame.
    //m_fenceValues[m_backBufferIndex] = currentFenceValue + 1;