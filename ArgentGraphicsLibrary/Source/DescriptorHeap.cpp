#include "../Inc/DescriptorHeap.h"

#include "../Inc/GraphicsDevice.h"

namespace argent::graphics
{
	DescriptorHeap::DescriptorHeap():
		descriptor_heap_object_(nullptr)
	,	heap_type_(HeapType::Count)
	,	max_descriptor_num_(0u)
	,	current_descriptor_counts_(0u)
	,	heap_handle_increment_size_(0u)
	,	cpu_handle_start_(0u)
	,	gpu_handle_start_(0u)
	{}

	void DescriptorHeap::Awake(const GraphicsDevice& graphics_device, HeapType heap_type, UINT max_descriptor_num)
	{
		heap_type_ = heap_type;
		max_descriptor_num_ = max_descriptor_num;
		current_descriptor_counts_ = 0u;

		D3D12_DESCRIPTOR_HEAP_FLAGS heap_flags{};
		D3D12_DESCRIPTOR_HEAP_TYPE descriptor_heap_type{};

		switch (heap_type_)
		{
		case HeapType::CbvSrvUav:
			heap_flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			descriptor_heap_type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			break;
		case HeapType::Rtv:
			heap_flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			descriptor_heap_type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			break;
		case HeapType::Dsv:
			heap_flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			descriptor_heap_type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			break;
		case HeapType::Smp:
			heap_flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			descriptor_heap_type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			break;
		case HeapType::Count:
			_ASSERT_EXPR(FALSE, L"Specified Invalied Descriptor Heap Type");
		}

		graphics_device.CreateDescriptorHeap(descriptor_heap_object_.ReleaseAndGetAddressOf(), heap_flags, 
			descriptor_heap_type, max_descriptor_num_);

		heap_handle_increment_size_ =  graphics_device.GetDescriptorHandleIncrementSize(descriptor_heap_type);
		cpu_handle_start_ = descriptor_heap_object_->GetCPUDescriptorHandleForHeapStart();
		if(heap_type_ == HeapType::CbvSrvUav || heap_type_ == HeapType::Smp)
		{
			gpu_handle_start_ = descriptor_heap_object_->GetGPUDescriptorHandleForHeapStart();
		}
	}

	Descriptor DescriptorHeap::PopDescriptor()
	{
		Descriptor ret{};
		ret.heap_index_ = current_descriptor_counts_;
		ret.cpu_handle_ = D3D12_CPU_DESCRIPTOR_HANDLE(cpu_handle_start_.ptr + static_cast<SIZE_T>(heap_handle_increment_size_) * current_descriptor_counts_);
		ret.gpu_handle_ = D3D12_GPU_DESCRIPTOR_HANDLE(gpu_handle_start_.ptr + static_cast<UINT64>(heap_handle_increment_size_) * current_descriptor_counts_);
		ret.p_descriptor_heap_ = this;

		++current_descriptor_counts_;

		return ret;
	}
}
