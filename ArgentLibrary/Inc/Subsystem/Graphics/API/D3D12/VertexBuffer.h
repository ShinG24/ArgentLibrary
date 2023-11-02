#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "GraphicsDevice.h"

namespace argent::graphics::dx12
{
	/**
	 * \brief 頂点バッファラップクラス
	 * テンプレートを使用していないため、コンストラクタで渡すことになる
	 * Structure Sizeに注意　
	 */
	class VertexBuffer
	{
	public:

		/**
		 * \brief コンストラクタ
		 * \param graphics_device Graphics Device 
		 * \param p_data データへのポインタ
		 * \param structure_size 構造体のサイズ
		 * \param vertex_counts 頂点数
		 * \param allow_update データのアップデートをするかどうか
		 */
		VertexBuffer(const GraphicsDevice* graphics_device, const void* p_data, size_t structure_size, size_t vertex_counts, bool allow_update = false);

		/**
		 * \brief デストラクタ
		 * コンストラクタでallow_update == trueとした場合、Unmap()をCallしないので
		 * 最後にCallする
		 */
		~VertexBuffer()
		{
			if(!is_unmapped_) buffer_object_->Unmap(0u, nullptr);	
		}

		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer(const VertexBuffer&&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
		VertexBuffer& operator=(const VertexBuffer&&) = delete;

		/**
		 * \brief Buffer Viewを取得
		 * \return D3D12_VERTEX_BUFFER_VIEW
		 */
		D3D12_VERTEX_BUFFER_VIEW GetView() const { return buffer_view_; }

		//TODO 名前おかしくない
		/**
		 * \brief 頂点数を取得
		 * \return Vertex Counts
		 */
		UINT GetVertexCounts() const { return buffer_view_.SizeInBytes / buffer_view_.StrideInBytes; }

		/**
		 * \brief Buffer Objectを取得
		 * \return ID3D12Resource
		 */
		ID3D12Resource* GetBufferObject() const { return buffer_object_.Get(); }

		/**
		 * \brief BufferのGpuAddressを取得
		 * \return D3D12_GPU_VIRTUAL_ADDRESS
		 */
		D3D12_GPU_VIRTUAL_ADDRESS GetBufferLocation() const { return buffer_object_->GetGPUVirtualAddress(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12Resource> buffer_object_{};
		D3D12_VERTEX_BUFFER_VIEW buffer_view_{};
		void* mapped_data_{};
		bool is_unmapped_;

	};
}
