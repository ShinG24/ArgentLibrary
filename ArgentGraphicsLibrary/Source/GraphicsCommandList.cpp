#include "../Inc/GraphicsCommandList.h"

namespace argent::graphics
{
	void GraphicsCommandList::OnAwake(ID3D12Device* device)
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
		}
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
}
