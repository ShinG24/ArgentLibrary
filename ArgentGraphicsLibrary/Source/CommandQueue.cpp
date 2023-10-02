#include "../Inc/CommandQueue.h"

namespace argent::graphics
{
	void CommandQueue::Awake(ID3D12Device* device, LPCWSTR name, D3D12_COMMAND_LIST_TYPE command_list_type,
		D3D12_COMMAND_QUEUE_PRIORITY command_queue_priority,
		D3D12_COMMAND_QUEUE_FLAGS command_queue_flags, UINT node_mask)
	{
		const D3D12_COMMAND_QUEUE_DESC command_queue_desc{ command_list_type, command_queue_priority, command_queue_flags, node_mask };
		const HRESULT hr = device->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(command_queue_object_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create ID3D12CommandQueue");
		command_queue_object_->SetName(name);
	}
}
