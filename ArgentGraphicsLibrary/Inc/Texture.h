#pragma once

#include <d3d12.h>
#include <wrl.h>


namespace argent::graphics
{
	class CommandQueue;
	class GraphicsDevice;
	class DescriptorHeap;

	class Texture
	{
	public:
		Texture(const GraphicsDevice* graphics_device, const CommandQueue* command_queue,
			DescriptorHeap* cbv_srv_uav_heap);
		~Texture() = default;

		Texture(const Texture&) = delete;
		Texture(const Texture&&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture& operator=(const Texture&&) = delete;

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_object_;
	};
}