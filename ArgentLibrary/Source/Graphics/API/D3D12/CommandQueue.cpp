#include "Graphics/API/D3D12/CommandQueue.h"

#include "Graphics/API/D3D12/GraphicsCommandList.h"


namespace argent::graphics::dx12
{
	CommandQueue::CommandQueue():
		command_queue_object_(nullptr)
	,	fence_object_(nullptr)
	,	fence_values_{0u}
	,	last_fence_value_(0u)
	,	event_(nullptr)
	{
	}

	void CommandQueue::Awake(ID3D12Device* device, LPCWSTR name, D3D12_COMMAND_LIST_TYPE command_list_type,
	                         D3D12_COMMAND_QUEUE_PRIORITY command_queue_priority,
	                         D3D12_COMMAND_QUEUE_FLAGS command_queue_flags, UINT node_mask)
	{
		const D3D12_COMMAND_QUEUE_DESC command_queue_desc{ command_list_type, command_queue_priority, command_queue_flags, node_mask };
		const HRESULT hr = device->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(command_queue_object_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create ID3D12CommandQueue");
		command_queue_object_->SetName(name);

		device->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_object_.ReleaseAndGetAddressOf()));
		fence_values_[0] = fence_values_[1] = fence_values_[2] = last_fence_value_ = 0u;

		event_.Attach(CreateEvent(nullptr, false, false, nullptr));
	}

	void CommandQueue::Execute(UINT num_command_lists, ID3D12CommandList* command_lists[]) const
	{
		command_queue_object_->ExecuteCommandLists(num_command_lists, command_lists);
	}

	void CommandQueue::Signal(UINT back_buffer_index)
	{
		command_queue_object_->Signal(fence_object_.Get(), ++last_fence_value_);
		fence_values_[back_buffer_index] = last_fence_value_;
	}

	void CommandQueue::Signal()
	{
		command_queue_object_->Signal(fence_object_.Get(), ++last_fence_value_);
	}

	void CommandQueue::WaitForGpu(UINT back_buffer_index) const
	{
		if(fence_object_->GetCompletedValue() < fence_values_[back_buffer_index])
		{
			HRESULT hr = fence_object_->SetEventOnCompletion(fence_values_[back_buffer_index], event_.Get());
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Set Event");
			WaitForSingleObjectEx(event_.Get(), INFINITE, false);
		}
	}

	void CommandQueue::WaitForGpu() const
	{
		if (fence_object_->GetCompletedValue() < last_fence_value_)
		{
			HRESULT hr = fence_object_->SetEventOnCompletion(last_fence_value_, event_.Get());
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Set Event");
			WaitForSingleObjectEx(event_.Get(), INFINITE, false);
		}
	}
}
