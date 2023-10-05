#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "DescriptorHeap.h"

namespace argent::graphics
{
	class GraphicsDevice;
	class SwapChain;
	class GraphicsCommandList;

	class FrameResource
	{
	public:
		FrameResource() = default;
		~FrameResource() = default;

		FrameResource(FrameResource&) = delete;
		FrameResource(FrameResource&&) = delete;
		FrameResource& operator=(FrameResource&) = delete;
		FrameResource& operator=(FrameResource&&) = delete;

		void Awake(const GraphicsDevice& graphics_device, const SwapChain& swap_chain, UINT back_buffer_index,
			const Descriptor& rtv_descriptor, const Descriptor& dsv_descriptor);
		void Activate(const GraphicsCommandList& command_list) const;
		void Deactivate(const GraphicsCommandList& command_list) const;

		ID3D12Resource* GetBackBuffer() const { return resource_object_.Get(); }
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_object_;
		Microsoft::WRL::ComPtr<ID3D12Resource> depth_buffer_;
		Descriptor rtv_descriptor_;
		Descriptor dsv_descriptor_;
		UINT back_buffer_index_;
	};
}
