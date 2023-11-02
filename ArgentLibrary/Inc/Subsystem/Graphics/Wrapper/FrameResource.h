#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "Subsystem/Graphics/API/D3D12/DescriptorHeap.h"

namespace argent::graphics
{
	namespace dx12
	{
		class GraphicsDevice;
		class GraphicsCommandList;
	}

	class SwapChain;
	struct GraphicsContext;

}
namespace argent::graphics
{
	//TODO こいつにコンスタントバッファをもたせる??
	/**
	 * \brief 最終的な描画結果を格納するオブジェクトを保持するクラス
	 * レンダーターゲット、深度バッファを持つ
	 */
	class FrameResource
	{
	public:

		FrameResource() = default;
		~FrameResource() = default;

		FrameResource(FrameResource&) = delete;
		FrameResource(FrameResource&&) = delete;
		FrameResource& operator=(FrameResource&) = delete;
		FrameResource& operator=(FrameResource&&) = delete;

		/**
		 * \brief 初期化 スワップチェーンからバックバッファを取得する。
		 * レンダーターゲットビュー、深度ステンシルビューを作る 
		 * \param graphics_context Graphics Context
		 * \param back_buffer_index Back Buffer Index
		 */
		void Awake(const GraphicsContext* graphics_context, UINT back_buffer_index);
		void Activate(const dx12::GraphicsCommandList* command_list) const;
		void Deactivate(const dx12::GraphicsCommandList* command_list) const;

		ID3D12Resource* GetBackBuffer() const { return resource_object_.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12Resource> resource_object_{};
		Microsoft::WRL::ComPtr<ID3D12Resource> depth_buffer_{};
		dx12::Descriptor rtv_descriptor_{};
		dx12::Descriptor dsv_descriptor_{};
		UINT back_buffer_index_{};
	};
}
