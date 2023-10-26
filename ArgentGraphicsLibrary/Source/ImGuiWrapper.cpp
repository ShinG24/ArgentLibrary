#include "../Inc/ImGuiWrapper.h"

#include "../External/Imgui/imgui.h"
#include "../External/Imgui/imgui_impl_dx12.h"
#include "../External/Imgui/imgui_impl_win32.h"

#include "../Inc/GraphicsCommon.h"

#include "../Inc/GraphicsDevice.h"
#include "../Inc/DescriptorHeap.h"


namespace argent::graphics
{
	void ImGuiWrapper::Awake(const dx12::GraphicsDevice* graphics_device, dx12::DescriptorHeap* cbv_srv_uav_heap, HWND hwnd)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		//Add Font file
		io.Fonts->AddFontFromFileTTF("Assets/Fonts/HGRMB.TTC", 16.0f, nullptr);

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		//	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGui::StyleColorsLight();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_ModalWindowDimBg].w = 1.0f;
		}
		style.Colors[ImGuiCol_WindowBg].w = 0.7f;
		style.Colors[ImGuiCol_FrameBg].w = 0.7f;
		style.Colors[ImGuiCol_TitleBgActive].w = 0.7f;


		font_descriptor_ = cbv_srv_uav_heap->PopDescriptor();
		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX12_Init(graphics_device->GetDevice(), kNumBackBuffers, DXGI_FORMAT_R8G8B8A8_UNORM,
			cbv_srv_uav_heap->GetDescriptorHeapObject(), font_descriptor_.cpu_handle_,
			font_descriptor_.gpu_handle_);
	}

	void ImGuiWrapper::Shutdown()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiWrapper::FrameBegin()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_::ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(300.0f, 500.0f), ImGuiCond_::ImGuiCond_Once);
	}

	void ImGuiWrapper::FrameEnd(ID3D12GraphicsCommandList* command_list)
	{
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list);
	}
}
