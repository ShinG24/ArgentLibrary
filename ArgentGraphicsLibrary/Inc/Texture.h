#pragma once

#include <d3d12.h>
#include <future>
#include <wrl.h>

#include "DescriptorHeap.h"

namespace argent::graphics::dx12
{
	class CommandQueue;
	class GraphicsDevice;
}

namespace argent::graphics
{
	

	class Texture
	{
	public:
		Texture(const dx12::GraphicsDevice* graphics_device, const dx12::CommandQueue* command_queue,
			dx12::DescriptorHeap* cbv_srv_uav_heap, const wchar_t * filename);
		~Texture() = default;

		Texture(const Texture&) = delete;
		Texture(const Texture&&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture& operator=(const Texture&&) = delete;

		void WaitBeforeUse();
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return descriptor_.gpu_handle_; }

	private:

		std::future<void> wait_for_finish_upload_;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_object_;
		dx12::Descriptor descriptor_;
		bool need_to_wait_ = true;
	};
}
