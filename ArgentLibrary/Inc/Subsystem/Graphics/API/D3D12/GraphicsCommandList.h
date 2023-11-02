#pragma once

#include <d3d12.h>
#include <wrl.h>

namespace argent::graphics::dx12
{
	/**
	 * \brief 	ID3D12GraphicsCommandListのWrapクラス
	 * 通常の描画用のコマンドリストに加え、
	 * レイトレーシング用のオブジェクトも持っている
	 */
	class GraphicsCommandList
	{
	public:

		GraphicsCommandList() = default;
		~GraphicsCommandList() = default;

		GraphicsCommandList(GraphicsCommandList&) = delete;
		GraphicsCommandList(GraphicsCommandList&&) = delete;
		GraphicsCommandList& operator=(GraphicsCommandList&) = delete;
		GraphicsCommandList& operator=(GraphicsCommandList&&) = delete;

		/**
		 * \brief 初期化
		 * \param device ID3D12Device 
		 */
		void Awake(ID3D12Device* device);

		/**
		 * \brief コマンドリストを有効化
		 */
		void Activate();

		/**
		 * \brief コマンドリストを閉じる
		 */
		void Deactivate();

		/**
		 * \brief コマンドアロケータ、コマンドリストオブジェクトのリセット
		 */
		void Reset();

		/**
		 * \brief コマンドリストの命令発行をストップ
		 */
		void Close();


		/**
		 * \brief レンダーターゲットビューをクリア
		 * \param rtv_cpu_handle Cpu Handle
		 * \param clear_color カラー
		 * \param num_rects 0でいいです
		 * \param p_rects Nullptrでいいです
		 */
		void ClearRtv(D3D12_CPU_DESCRIPTOR_HANDLE rtv_cpu_handle, 
		              float clear_color[4], UINT num_rects = 0, const RECT* p_rects = nullptr) const;

		/**
		 * \brief デプスステンシルビューをクリア
		 * \param dsv_cpu_handle Cpu Handle
		 * \param clear_flags DepthとStencilどっちをクリアするか（あるいは両方）
		 * \param depth 深度クリアバリュー 基本的に1
		 * \param stencil ステンシルクリアバリュー　基本的に0
		 * \param num_rects 0でいいです
		 * \param p_rects nullptrでいいです
		 */
		void ClearDsv(D3D12_CPU_DESCRIPTOR_HANDLE dsv_cpu_handle,
		              D3D12_CLEAR_FLAGS clear_flags = D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		              float depth = 1.0, UINT8 stencil = 0, UINT num_rects = 0, 
		              const D3D12_RECT* p_rects = nullptr) const;

		/**
		 * \brief Transition Resource Barrierをセット
		 * \param p_resource バリアをセットするリソースのポインタ
		 * \param state_before 現在のリソースステート
		 * \param state_after バリア後のリソースステート
		 */
		void SetTransitionBarrier(ID3D12Resource* p_resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after) const;

		/**
		 * \brief Uav Resource Barrierをセット
		 * \param p_resource バリアをセットするリソースのポインタ
		 * \param flags たぶんNoneでいいと思う
		 */
		void SetUavBarrier(ID3D12Resource* p_resource, D3D12_RESOURCE_BARRIER_FLAGS flags) const;

		/**
		 * \brief レンダーターゲットをセット
		 * \param p_rtv_handles レンダーターゲットのCpu Handle
		 * \param p_dsv_handle 深度バッファのCpu Handle
		 */
		void SetRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE* p_rtv_handles, const D3D12_CPU_DESCRIPTOR_HANDLE* p_dsv_handle) const;

		/**
		 * \brief 複数のレンダーターゲットセット用
		 * \param num_render_targets レンダーターゲットの数
		 * \param p_rtv_handles レンダーターゲットのハンドルスタート
		 * \param in_a_row_handle よくわからんけどポインタが連続してるかどうか
		 * \param p_dsv_handle 深度ハンドル
		 */
		void SetRenderTargets(UINT num_render_targets,
		                      const D3D12_CPU_DESCRIPTOR_HANDLE* p_rtv_handles, bool in_a_row_handle, const D3D12_CPU_DESCRIPTOR_HANDLE* p_dsv_handle) const;

		/**
		 * \brief View Portをセット
		 * \param num_viewports ビューポートの数
		 * \param p_viewport ビューポートのポインタ
		 */
		void SetViewports(UINT num_viewports, const D3D12_VIEWPORT* p_viewport) const;

		/**
		 * \brief Rectをセット
		 * \param num_rects Rectの数
		 * \param p_rect Rectへのポインタ
		 */
		void SetRects(UINT num_rects, const D3D12_RECT* p_rect) const;

		ID3D12GraphicsCommandList* GetCommandList() const { return command_list_.Get(); }
		ID3D12GraphicsCommandList4* GetCommandList4() const { return command_list_.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> command_list_{};
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator_{};
		bool is_close_{};
		bool is_reset_{};

	};
}