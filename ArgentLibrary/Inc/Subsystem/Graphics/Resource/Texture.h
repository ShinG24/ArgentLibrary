#pragma once

#include <d3d12.h>
#include <future>
#include <wrl.h>

#include "../API/D3D12/DescriptorHeap.h"

namespace argent::graphics::dx12
{
	class CommandQueue;
	class GraphicsDevice;
}

namespace argent::graphics
{
	struct GraphicsContext;

	/**
	 * \brief Texture class
	 * 読み込んだ画像の幅高さなどを持っている
	 */
	class Texture
	{
	public:

		Texture(const GraphicsContext* graphics_context, const char* filename);
		~Texture() = default;

		Texture(const Texture&) = delete;
		Texture(const Texture&&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture& operator=(const Texture&&) = delete;

		/**
		 * \brief リソースをUploadし終わるまで待つ
		 */
		void WaitBeforeUse() const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return descriptor_.gpu_handle_; }

	private:

		std::future<void> wait_for_finish_upload_{};
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_object_{};
		dx12::Descriptor descriptor_{};
		bool need_to_wait_ = true;
		UINT width_{};
		UINT height_{};
	};
}
