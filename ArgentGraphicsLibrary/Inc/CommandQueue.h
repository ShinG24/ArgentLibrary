#pragma once

#include <d3d12.h>
#include <wrl.h>


namespace argent::graphics
{
	class Fence;

	//Wrap class of ID3D12CommandQueue
	//Some Fucntion are Wraped.
	class CommandQueue
	{
	public:
		CommandQueue() = default;
		~CommandQueue() = default;

		CommandQueue(CommandQueue&) = delete;
		CommandQueue(CommandQueue&&) = delete;
		CommandQueue& operator=(CommandQueue&) = delete;
		CommandQueue& operator=(CommandQueue&&) = delete;

		/**
		 * \brief Create ID3D12CommandQueueObject.
		 * \param device ID3D12Device object.
		 * \param name : Object name. Usefull for Debugging.
		 * \param command_list_type : command_list_type.
		 * \param command_queue_priority : the priority of this command_queu.
		 * \param command_queue_flags : do not specify expect trying some unique program
		 * \param node_mask specify 0 if you use single GPU.
		 */
		void Awake(ID3D12Device* device, LPCWSTR name, D3D12_COMMAND_LIST_TYPE command_list_type = D3D12_COMMAND_LIST_TYPE_DIRECT, 
		           D3D12_COMMAND_QUEUE_PRIORITY command_queue_priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH, 
		           D3D12_COMMAND_QUEUE_FLAGS command_queue_flags = D3D12_COMMAND_QUEUE_FLAG_NONE, 
		           UINT node_mask = 0);

		/**
		 * \brief Execute Command List.
		 * \param num_command_lists : num command lists
		 * \param command_lists : pointer to the command list array.
		 */
		void Execute(UINT num_command_lists, ID3D12CommandList* command_lists[]) const;

		/**
		 * \brief Issue a Fecne.
		 * \param fence : fence
		 */
		void Signal(const Fence& fence) const;

		ID3D12CommandQueue* GetCommandQueue() const { return command_queue_object_.Get(); }
	private:
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue_object_;
	};
}
