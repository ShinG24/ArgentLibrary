#pragma once

#include <d3d12.h>
#include <wrl.h>


namespace argent::graphics
{
	class Fence;
	class CommandQueue
	{
	public:
		CommandQueue() = default;
		~CommandQueue() = default;

		CommandQueue(CommandQueue&) = delete;
		CommandQueue(CommandQueue&&) = delete;
		CommandQueue& operator=(CommandQueue&) = delete;
		CommandQueue& operator=(CommandQueue&&) = delete;

		void Awake(ID3D12Device* device, LPCWSTR name, D3D12_COMMAND_LIST_TYPE command_list_type = D3D12_COMMAND_LIST_TYPE_DIRECT, 
			D3D12_COMMAND_QUEUE_PRIORITY command_queue_priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH, 
			D3D12_COMMAND_QUEUE_FLAGS command_queue_flags = D3D12_COMMAND_QUEUE_FLAG_NONE, 
			UINT node_mask = 0);

		void Execute(UINT num_command_lists, ID3D12CommandList* command_lists[]) const;

		ID3D12CommandQueue* GetCommandQueue() const { return command_queue_object_.Get(); }
		void Signal(const Fence& fence) const;

	private:
	private:
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue_object_;
	};
}
