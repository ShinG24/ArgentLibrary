#include "Subsystem/Graphics/API/D3D12/GraphicsCommandList.h"

namespace argent::graphics::dx12
{
	void GraphicsCommandList::Awake(ID3D12Device* device)
	{
		is_close_ = is_reset_ = false;

		HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(command_allocator_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create ID3D12CommanAllocator");

		Microsoft::WRL::ComPtr<ID3D12CommandList> tmp_command_list;
		hr = device->CreateCommandList(0u, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator_.Get(), nullptr,
			IID_PPV_ARGS(tmp_command_list.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create ID3D12CommandList");

		hr = tmp_command_list->QueryInterface(IID_PPV_ARGS(command_list_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Query Interface ID3D12CommandList to ID3D12CommandList6");

		Close();
		Reset();
	}

	void GraphicsCommandList::Activate()
	{
		Reset();
	}

	void GraphicsCommandList::Deactivate()
	{
		Close();
	}


	void GraphicsCommandList::Reset()
	{
		if(is_close_ && !is_reset_)
		{
			command_allocator_.Get()->Reset();
			command_list_.Get()->Reset(command_allocator_.Get(), nullptr);
			is_close_ = false;
			is_reset_ = true;
		}
	}

	void GraphicsCommandList::Close()
	{
		if(!is_close_)
		{
			command_list_->Close();
			is_close_ = true;
			is_reset_ = false;
		}
	}

	void GraphicsCommandList::ClearRtv(D3D12_CPU_DESCRIPTOR_HANDLE rtv_cpu_handle, 
		float clear_color[4], UINT num_rects, const RECT* p_rects) const
	{
		command_list_->ClearRenderTargetView(rtv_cpu_handle, clear_color, num_rects, p_rects);
	}

	void GraphicsCommandList::ClearDsv(D3D12_CPU_DESCRIPTOR_HANDLE dsv_cpu_handle,
		D3D12_CLEAR_FLAGS clear_flags, float depth, UINT8 stencil, UINT num_rects, 
		const D3D12_RECT* p_rects) const
	{
		command_list_->ClearDepthStencilView(dsv_cpu_handle, clear_flags, depth, stencil, num_rects, p_rects);
	}

	void GraphicsCommandList::SetTransitionBarrier(ID3D12Resource* p_resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after) const
	{
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Flags =D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.StateBefore = state_before;
		barrier.Transition.StateAfter = state_after;
		barrier.Transition.pResource = p_resource;
		command_list_->ResourceBarrier(1u, &barrier);
	}

	void GraphicsCommandList::SetUavBarrier(ID3D12Resource* p_resource, D3D12_RESOURCE_BARRIER_FLAGS flags) const
	{
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.UAV.pResource = p_resource;
		barrier.Flags = flags;
		command_list_->ResourceBarrier(1u, &barrier);
	}

	void GraphicsCommandList::SetRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE* p_rtv_handles,
	                                          const D3D12_CPU_DESCRIPTOR_HANDLE* p_dsv_handle) const
	{
		command_list_->OMSetRenderTargets(1, p_rtv_handles, false, p_dsv_handle);
	}

	void GraphicsCommandList::SetRenderTargets(UINT num_render_targets,
		const D3D12_CPU_DESCRIPTOR_HANDLE* p_rtv_handles, bool in_a_row_handle, const D3D12_CPU_DESCRIPTOR_HANDLE* p_dsv_handle) const
	{
		command_list_->OMSetRenderTargets(num_render_targets, p_rtv_handles, in_a_row_handle, p_dsv_handle);
	}

	void GraphicsCommandList::SetViewports(UINT num_viewports, const D3D12_VIEWPORT* p_viewport) const
	{
		command_list_->RSSetViewports(num_viewports, p_viewport);
	}

	void GraphicsCommandList::SetRects(UINT num_rects, const D3D12_RECT* p_rect) const
	{
		command_list_->RSSetScissorRects(num_rects, p_rect);
	}
}
