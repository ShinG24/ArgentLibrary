#include "../Inc/FrameResource.h"

#include "../Inc/GraphicsDevice.h"
#include "../Inc/SwapChain.h"
#include "../Inc/DescriptorHeap.h"
#include "../Inc/GraphicsCommandList.h"

namespace argent::graphics
{
	void FrameResource::Awake(const GraphicsDevice& graphics_device, const SwapChain& swap_chain,
		UINT back_buffer_index, const Descriptor& rtv_descriptor, const Descriptor& dsv_descriptor)
	{
		back_buffer_index_ = back_buffer_index;
		rtv_descriptor_ = rtv_descriptor;
		dsv_descriptor_ = dsv_descriptor;

		swap_chain.GetBuffer(back_buffer_index_, resource_object_.ReleaseAndGetAddressOf());

		graphics_device.CreateRTV(resource_object_.Get(), rtv_descriptor_.cpu_handle_);
	}

	void FrameResource::Activate(const GraphicsCommandList& command_list) const
	{
		float clear_color[4]{0, 1, 0, 1};

		command_list.SetTransitionBarrier(resource_object_.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		command_list.ClearRtv(rtv_descriptor_.cpu_handle_, clear_color);
		command_list.SetRenderTarget(&rtv_descriptor_.cpu_handle_, nullptr);
	}

	void FrameResource::Deactivate(const GraphicsCommandList& command_list) const
	{
		command_list.SetTransitionBarrier(resource_object_.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
}
