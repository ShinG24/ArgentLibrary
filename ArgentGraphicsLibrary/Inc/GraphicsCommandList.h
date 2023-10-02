#pragma once

#include <d3d12.h>
#include <wrl.h>

namespace argent::graphics
{
	class GraphicsCommandList
	{
	public:
		GraphicsCommandList() = default;
		~GraphicsCommandList() = default;

		void OnAwake(ID3D12Device* device);
		void Reset();
		void Close();

		void SetRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE* p_rtv_handles, const D3D12_CPU_DESCRIPTOR_HANDLE* p_dsv_handle) const;
		void SetRenderTargets(UINT num_render_targets,
		const D3D12_CPU_DESCRIPTOR_HANDLE* p_rtv_handles, bool in_a_row_handle, const D3D12_CPU_DESCRIPTOR_HANDLE* p_dsv_handle) const;
	private:
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> command_list_;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator_;
		bool is_close_;
		bool is_reset_;
	};
}