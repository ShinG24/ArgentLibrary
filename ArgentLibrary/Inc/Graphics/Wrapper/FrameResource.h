#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "Graphics/API/D3D12/DescriptorHeap.h"

namespace argent::graphics
{
	namespace dx12
	{
		class GraphicsDevice;
		class GraphicsCommandList;
	}

	class SwapChain;
}
namespace argent::graphics
{

	class FrameResource
	{
	public:
		FrameResource() = default;
		~FrameResource() = default;

		FrameResource(FrameResource&) = delete;
		FrameResource(FrameResource&&) = delete;
		FrameResource& operator=(FrameResource&) = delete;
		FrameResource& operator=(FrameResource&&) = delete;

		void Awake(const dx12::GraphicsDevice* graphics_device, const SwapChain* swap_chain, 
			UINT back_buffer_index, const dx12::Descriptor& rtv_descriptor, 
			const dx12::Descriptor& dsv_descriptor);
		void Activate(const dx12::GraphicsCommandList* command_list) const;
		void Deactivate(const dx12::GraphicsCommandList* command_list) const;

		ID3D12Resource* GetBackBuffer() const { return resource_object_.Get(); }
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_object_;
		Microsoft::WRL::ComPtr<ID3D12Resource> depth_buffer_;
		dx12::Descriptor rtv_descriptor_;
		dx12::Descriptor dsv_descriptor_;
		UINT back_buffer_index_;
	};
}
