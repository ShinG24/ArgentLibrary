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

		GraphicsCommandList(GraphicsCommandList&) = delete;
		GraphicsCommandList(GraphicsCommandList&&) = delete;
		GraphicsCommandList& operator=(GraphicsCommandList&) = delete;
		GraphicsCommandList& operator=(GraphicsCommandList&&) = delete;

		void Awake(ID3D12Device* device);

		void Activate();
		void Deactivate();
		void Reset();
		void Close();

		void ClearRtv(D3D12_CPU_DESCRIPTOR_HANDLE rtv_cpu_handle, 
		float clear_color[4], UINT num_rects = 0, const RECT* p_rects = nullptr) const;
		void ClearDsv(D3D12_CPU_DESCRIPTOR_HANDLE dsv_cpu_handle,
		D3D12_CLEAR_FLAGS clear_flags = D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			float depth = 1.0, UINT8 stencil = 0, UINT num_rects = 0, 
			const D3D12_RECT* p_rects = nullptr) const;
		void SetTransitionBarrier(ID3D12Resource* p_resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after) const;
		void SetRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE* p_rtv_handles, const D3D12_CPU_DESCRIPTOR_HANDLE* p_dsv_handle) const;
		void SetRenderTargets(UINT num_render_targets,
		const D3D12_CPU_DESCRIPTOR_HANDLE* p_rtv_handles, bool in_a_row_handle, const D3D12_CPU_DESCRIPTOR_HANDLE* p_dsv_handle) const;


		ID3D12GraphicsCommandList* GetCommandList() const { return command_list_.Get(); }
	private:
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> command_list_;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator_;
		bool is_close_;
		bool is_reset_;
	};
}