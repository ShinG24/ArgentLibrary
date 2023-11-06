#pragma once

#include <d3d12.h>
#include <wrl.h>


namespace argent::graphics::dx12
{
	class GraphicsDevice;

	/**
	 * \brief Index Bufferクラス
	 * データのレイアウトはUint32固定なので注意
	 */
	class IndexBuffer
	{
	public:

		/**
		 * \brief コンストラクタ
		 * \param graphics_device Graphics Device 
		 * \param p_data Index Dataへのポインタ
		 * \param index_counts インデックス数
		 */
		IndexBuffer(const GraphicsDevice* graphics_device, const UINT32* p_data, UINT index_counts);

		~IndexBuffer() = default;

		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer(const IndexBuffer&&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;
		IndexBuffer& operator=(const IndexBuffer&&) = delete;

		/**
		 * \brief Buffer Viewを取得
		 * \return D3D12_INDEX_BUFFER_VIEW
		 */
		D3D12_INDEX_BUFFER_VIEW GetView() const { return buffer_view_; }

		/**
		 * \brief Buffer Viewのポインタを取得
		 * \return D3D12_INDEX_BUFFER_VIEW Pointer
		 */
		const D3D12_INDEX_BUFFER_VIEW* GetViewPointer() const { return &buffer_view_; }

		/**
		 * \brief Bufferのポインタを取得
		 * \return ID3D12Resource
		 */
		ID3D12Resource* GetBufferObject() const { return buffer_object_.Get(); }

		//TODO この関数名前へんじゃね
		/**
		 * \brief インデックスの数を取得
		 * \return Index Counts
		 */
		UINT GetIndexCounts() const { return index_counts_; }

	private:

		Microsoft::WRL::ComPtr<ID3D12Resource> buffer_object_{};
		D3D12_INDEX_BUFFER_VIEW buffer_view_{};
		UINT index_counts_;
	};
}