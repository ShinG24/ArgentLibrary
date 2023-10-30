#pragma once

#include "../API/D3D12/DescriptorHeap.h"

namespace argent::graphics
{

	class ImGuiWrapper
	{
	public:
		ImGuiWrapper() = default;
		~ImGuiWrapper() = default;

		ImGuiWrapper(const ImGuiWrapper&) = delete;
		ImGuiWrapper(const ImGuiWrapper&&) = delete;
		ImGuiWrapper& operator=(const ImGuiWrapper&) = delete;
		ImGuiWrapper& operator=(const ImGuiWrapper&&) = delete;

		void Awake(const dx12::GraphicsDevice* graphics_device, dx12::DescriptorHeap* cbv_srv_uav_heap, HWND hwnd);
		void Shutdown();

		void FrameBegin();
		void FrameEnd(ID3D12GraphicsCommandList* command_list);

	private:

		dx12::Descriptor font_descriptor_;
	};

}
