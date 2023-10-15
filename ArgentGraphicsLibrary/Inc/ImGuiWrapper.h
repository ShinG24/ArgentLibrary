#pragma once
#include "DescriptorHeap.h"

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

		void Awake(const GraphicsDevice* graphics_device, DescriptorHeap* cbv_srv_uav_heap, HWND hwnd);
		void Shutdown();

		void FrameBegin();
		void FrameEnd(ID3D12GraphicsCommandList* command_list);

	private:

		Descriptor font_descriptor_;
	};

}
