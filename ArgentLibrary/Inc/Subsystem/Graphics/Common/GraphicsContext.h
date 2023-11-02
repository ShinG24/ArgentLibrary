#pragma once

namespace argent::graphics
{
	class SwapChain;
}
namespace argent::graphics::dx12
{
	class GraphicsDevice;
	class CommandQueue;
	class GraphicsCommandList;
	class DescriptorHeap;
	class AccelerationStructureManager;
}

namespace argent::graphics
{
	/**
	 * \brief APIWrap�N���X�ȊO�̃��\�[�X�쐬�Ɏg��(Model�Ƃ�Mataerial�Ƃ�Texture�Ƃ�)
	 */
	struct GraphicsContext
	{
		dx12::GraphicsDevice* graphics_device_;
		SwapChain* swap_chain_;
		dx12::CommandQueue* resource_upload_command_queue_;
		dx12::GraphicsCommandList* resource_upload_command_list_;
		dx12::DescriptorHeap* cbv_srv_uav_descriptor_heap_;
		dx12::DescriptorHeap* rtv_descriptor_;
		dx12::DescriptorHeap* dsv_descriptor_;
		dx12::AccelerationStructureManager* as_manager_;
	};
}