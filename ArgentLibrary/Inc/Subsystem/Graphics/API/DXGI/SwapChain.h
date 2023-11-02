#pragma once

#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>


namespace argent::graphics
{
	//TODO ウィンドウがリサイズされたとき用の関数を作る
	/**
	 * \brief スワップチェーンのラップクラス
	 * プレゼント命令はこいつでやります
	 */
	class SwapChain
	{
	public:

		SwapChain() = default;
		~SwapChain() = default;

		SwapChain(const SwapChain&) = delete;
		SwapChain(const SwapChain&&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&&) = delete;

		/**
		 * \brief 初期化関数
		 * \param hwnd Window Handle
		 * \param dxgi_factory Factory
		 * \param command_queue Command Queue
		 * \param num_back_buffers バックバッファ数
		 */
		void Awake(HWND hwnd, IDXGIFactory6* dxgi_factory, ID3D12CommandQueue* command_queue, UINT num_back_buffers);

		/**
		 * \brief バックバッファを入れ替える
		 */
		void Present() const;

		/**
		 * \brief 最初にレンダーターゲットを作成するときに使うよう
		 * Back Bufferを取得する関数
		 * \param buffer_index Back Buffer Index
		 * \param pp_resource ID3D12Resource
		 * \return Hresult
		 */
		HRESULT GetBuffer(UINT buffer_index, ID3D12Resource** pp_resource) const;

		/**
		 * \brief 現在のバックバッファインデックスを取得
		 * \return Current Back Buffer Index
		 */
		UINT GetCurrentBackBufferIndex() const;

		/**
		 * \brief スワップチェーンバッファの幅
		 * \return Width
		 */
		UINT64 GetWidth() const { return buffer_width_; }

		/**
		 * \brief スワップチェーンバッファの高さ
		 * \return Height
		 */
		UINT GetHeight() const { return buffer_height_; }

	private:

		Microsoft::WRL::ComPtr<IDXGISwapChain4> swap_chain_object_;
		UINT64 buffer_width_;
		UINT buffer_height_;
	};
}