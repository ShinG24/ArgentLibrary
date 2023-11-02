#pragma once

#include <d3d12.h>
#include <wrl.h>

namespace argent::graphics::dx12
{
	class GraphicsDevice;

	class DescriptorHeap;

	struct Descriptor
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle_;	
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle_;
		UINT heap_index_;
		DescriptorHeap* p_descriptor_heap_;
	};

	/**
	 * \brief DescriptorHeapクラス
	 * Awakeで設定された数だけDescriptorをPopできる
	 * 最大数を超えるとアサートで落ちるので注意
	 */
	class DescriptorHeap
	{
	public:
		/**
		 * \brief DescriptorHeap Type
		 */
		enum class HeapType
		{
			CbvSrvUav,
			Rtv,
			Dsv,
			Smp,
			Count,
		};

	public:

		DescriptorHeap();
		~DescriptorHeap() = default;

		DescriptorHeap(DescriptorHeap&) = delete;
		DescriptorHeap(DescriptorHeap&&) = delete;
		DescriptorHeap& operator=(DescriptorHeap&) = delete;
		DescriptorHeap& operator=(DescriptorHeap&&) = delete;

		/**
		 * \brief 初期化処理
		 * \param graphics_device GraphicsDevice
		 * \param heap_type Heap Type
		 * \param max_descriptor_num ディスクリプタの最大数  
		 */
		void Awake(const GraphicsDevice* graphics_device, HeapType heap_type, UINT max_descriptor_num);

		Descriptor PopDescriptor();

		/**
		 * \brief ヒープタイプの取得
		 * \return Heap Type
		 */
		HeapType GetHeapType() const { return heap_type_; }

		/**
		 * \brief ディスクリプタの最大数を取得
		 * \return Max Descriptor Num
		 */
		UINT GetMaxDescriptorNum() const { return max_descriptor_num_; }

		/**
		 * \brief 現在発行しているディスクリプタの数
		 * \return Curernt Descriptor Counts
		 */
		UINT CurrentDescriptorCounts() const { return current_descriptor_counts_; }

		/**
		 * \brief Descriptor Objectのポインタ
		 * \return ID3D12DescriptorHeap
		 */
		ID3D12DescriptorHeap* GetDescriptorHeapObject() const { return descriptor_heap_object_.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_object_;
		HeapType heap_type_;
		UINT max_descriptor_num_;
		UINT current_descriptor_counts_;
		UINT heap_handle_increment_size_;
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle_start_;
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle_start_;

	};
}
