#include "../Inc/FrameResource.h"

#include "../Inc/GraphicsDevice.h"
#include "../Inc/SwapChain.h"
#include "../Inc/DescriptorHeap.h"
#include "../Inc/GraphicsCommandList.h"

namespace argent::graphics
{
	void FrameResource::Awake(const dx12::GraphicsDevice& graphics_device, const SwapChain& swap_chain,
		UINT back_buffer_index, const dx12::Descriptor& rtv_descriptor, const dx12::Descriptor& dsv_descriptor)
	{
		back_buffer_index_ = back_buffer_index;
		rtv_descriptor_ = rtv_descriptor;
		dsv_descriptor_ = dsv_descriptor;

		swap_chain.GetBuffer(back_buffer_index_, resource_object_.ReleaseAndGetAddressOf());

		graphics_device.CreateRTV(resource_object_.Get(), rtv_descriptor_.cpu_handle_);

		HRESULT hr{ S_OK };
		D3D12_RESOURCE_DESC res_desc{};
		res_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		res_desc.Width = resource_object_->GetDesc().Width;
		res_desc.Height = resource_object_->GetDesc().Height;
		res_desc.DepthOrArraySize = 1;
		res_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		res_desc.SampleDesc.Count = 1u;
		res_desc.SampleDesc.Quality = 0u;
		res_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_HEAP_PROPERTIES heap_prop{};
		heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
		heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_CLEAR_VALUE depth_clear_value{};
		depth_clear_value.DepthStencil.Depth = 1.0f;
		depth_clear_value.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

		hr = graphics_device.GetDevice()->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
			&res_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depth_clear_value,
			IID_PPV_ARGS(depth_buffer_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Depth Buffer");

		D3D12_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
		depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depth_stencil_view_desc.Flags = D3D12_DSV_FLAG_NONE;
		depth_stencil_view_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		graphics_device.GetDevice()->CreateDepthStencilView(depth_buffer_.Get(), &depth_stencil_view_desc,
			dsv_descriptor.cpu_handle_);

		resource_object_->SetName(L"Frame Resource");
	}

	void FrameResource::Activate(const dx12::GraphicsCommandList& command_list) const
	{
		float clear_color[4]{0, 0, 0, 1};

		command_list.SetTransitionBarrier(resource_object_.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		command_list.ClearRtv(rtv_descriptor_.cpu_handle_, clear_color);
		command_list.ClearDsv(dsv_descriptor_.cpu_handle_);
		command_list.SetRenderTarget(&rtv_descriptor_.cpu_handle_, &dsv_descriptor_.cpu_handle_);
	}

	void FrameResource::Deactivate(const dx12::GraphicsCommandList& command_list) const
	{
		command_list.SetTransitionBarrier(resource_object_.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
}
