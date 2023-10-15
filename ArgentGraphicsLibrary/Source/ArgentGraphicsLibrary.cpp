#include "../Inc/ArgentGraphicsLibrary.h"

#include <windows.h>

#include "../External/d3dx12.h"

//Imgui
#include "../External/Imgui/imgui_impl_win32.h"
#include "../External/Imgui/imgui_impl_dx12.h"
#include "../External/Imgui/imgui.h"

#include "../Inc/ShaderCompiler.h"



#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3D12.lib")

namespace argent::graphics
{
	void GraphicsLibrary::Awake(HWND hwnd)
	{
		hwnd_ = hwnd;

#ifdef _DEBUG
		OnDebugLayer();
#endif

		dxgi_factory_.Awake();
		graphics_device_.Awake(dxgi_factory_.GetIDxgiFactory());
		main_rendering_queue_.Awake(graphics_device_.GetLatestDevice(), L"Main Rendering Queue");
		resource_upload_queue_.Awake(graphics_device_.GetLatestDevice(), L"Resource Upload Queue");
		swap_chain_.Awake(hwnd, dxgi_factory_.GetIDxgiFactory(), main_rendering_queue_.GetCommandQueue(), kNumBackBuffers);

		//Check Raytracing tier supported
		const bool raytracing_supported = graphics_device_.IsDirectXRaytracingSupported();
		_ASSERT_EXPR(raytracing_supported, L"DXR not Supported");

		CreateDeviceDependencyObjects();

		back_buffer_index_ = swap_chain_.GetCurrentBackBufferIndex();

		RECT rect{};
		GetClientRect(hwnd_, &rect);
		//GetWindowRect(hwnd_, &rect);
		viewport_ = D3D12_VIEWPORT(0.0f, 0.0f, static_cast<FLOAT>(rect.right - rect.left), 
			static_cast<FLOAT>(rect.bottom - rect.top), 0.0f, 1.0f);
		scissor_rect_ = D3D12_RECT(rect);

		//raster_renderer_.Awake(graphics_device_);

		resource_upload_command_list_.Activate();
#if _USE_RAY_TRACER_
		raytracer_.Awake(graphics_device_, resource_upload_command_list_,
			resource_upload_queue_, swap_chain_.GetWidth(), swap_chain_.GetHeight(),
			cbv_srv_uav_heap_);
#endif
		ImguiAwake();
	}

	void GraphicsLibrary::Shutdown()
	{
		ImguiShutdown();
		/*INT last_back_buffer_index = static_cast<INT>(back_buffer_index_) - 1;
		if(last_back_buffer_index < 0) { last_back_buffer_index = kNumBackBuffers; }
		fence_.WaitForGpu(last_back_buffer_index);*/
		main_rendering_queue_.WaitForGpu();
		resource_upload_queue_.WaitForGpu();

	}

	void GraphicsLibrary::FrameBegin()
	{
		main_rendering_queue_.WaitForGpu(back_buffer_index_);

		HRESULT hr = graphics_device_.GetLatestDevice()->GetDeviceRemovedReason();
		if(FAILED(hr))
		{
			_ASSERT_EXPR(FALSE, L"D3D12Device removed!!");
		}

		auto& command_list = graphics_command_list_[back_buffer_index_];
		command_list.Activate();

		frame_resources_[back_buffer_index_].Activate(command_list);

		command_list.SetViewports(1u, &viewport_);
		command_list.SetRects(1u, &scissor_rect_);

		std::vector heaps = { cbv_srv_uav_heap_.GetDescriptorHeapObject() };
		command_list.GetCommandList()->SetDescriptorHeaps(1u, heaps.data());

		//For ImGui
		ImguiFrameBegin();

		OnRender();
	}

	void GraphicsLibrary::FrameEnd()
	{
		ImguiFrameEnd();

		auto& command_list = graphics_command_list_[back_buffer_index_];

		frame_resources_[back_buffer_index_].Deactivate(command_list);

		command_list.Deactivate();

		ID3D12CommandList* command_lists[]
		{
			command_list.GetCommandList()
		};
		main_rendering_queue_.Execute(1u, command_lists, back_buffer_index_);
		

		//fence_.PutUpFence(main_rendering_queue_);
		//main_rendering_queue_.GetCommandQueue()->Signal(fence_.GetFence(), ++fence_value_);

		//if(fence_.GetFence()->GetCompletedValue() < fence_value_)
		//{
		//	HANDLE event_handler{};
		//	fence_.GetFence()->SetEventOnCompletion(fence_value_, event_handler);
		//	WaitForSingleObject(event_handler, INFINITE);
		//}

		swap_chain_.Present();
		back_buffer_index_ = swap_chain_.GetCurrentBackBufferIndex();
		
	}

	void GraphicsLibrary::OnRender()
	{
		const auto command_list = graphics_command_list_[back_buffer_index_].GetCommandList();
		if(on_raster_mode_)
		{
			//raster_renderer_.OnRender(command_list);
		}
		else
		{
#if _USE_RAY_TRACER_
			//raster_renderer_.OnRender(command_list);
			raytracer_.OnRender(graphics_command_list_[back_buffer_index_]);


			//return;
			D3D12_RESOURCE_BARRIER resource_barrier{};
			resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
			resource_barrier.Transition.pResource = frame_resources_[back_buffer_index_].GetBackBuffer();
			command_list->ResourceBarrier(1u, &resource_barrier);

			command_list->CopyResource(frame_resources_[back_buffer_index_].GetBackBuffer(), 
				raytracer_.GetOutputBuffer());

			resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			command_list->ResourceBarrier(1u, &resource_barrier);
#endif
		}
	}

	void GraphicsLibrary::CreateDeviceDependencyObjects()
	{
		cbv_srv_uav_heap_.Awake(graphics_device_, DescriptorHeap::HeapType::CbvSrvUav, 10000);
		rtv_heap_.Awake(graphics_device_, DescriptorHeap::HeapType::Rtv, 100);
		dsv_heap_.Awake(graphics_device_, DescriptorHeap::HeapType::Dsv, 100);
		smp_heap_.Awake(graphics_device_, DescriptorHeap::HeapType::Smp, 50);

		graphics_command_list_[0].Awake(graphics_device_.GetDevice());
		graphics_command_list_[1].Awake(graphics_device_.GetDevice());
		graphics_command_list_[2].Awake(graphics_device_.GetDevice());
		resource_upload_command_list_.Awake(graphics_device_.GetDevice());

		for(int i = 0; i < kNumBackBuffers; ++i)
		{
			frame_resources_[i].Awake(graphics_device_, swap_chain_, i, rtv_heap_.PopDescriptor(), dsv_heap_.PopDescriptor());
		}
		//Create Raytracing Objects
	}

	void GraphicsLibrary::OnDebugLayer() const
	{
		HRESULT hr{ S_OK };
		ID3D12Debug* debugLayer = nullptr;
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Valied D3d12 Debug Layer");

		debugLayer->EnableDebugLayer();
		debugLayer->Release();
	}

	void GraphicsLibrary::ImguiAwake()
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
		if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_ModalWindowDimBg].w = 1.0f;
		}
		style.Colors[ImGuiCol_WindowBg].w = 0.7f;
		style.Colors[ImGuiCol_FrameBg].w = 0.7f;
		style.Colors[ImGuiCol_TitleBgActive].w = 0.7f;


		imgui_font_srv_descriptor_ = cbv_srv_uav_heap_.PopDescriptor();
		ImGui_ImplWin32_Init(hwnd_);
		ImGui_ImplDX12_Init(graphics_device_.GetDevice(), kNumBackBuffers, DXGI_FORMAT_R8G8B8A8_UNORM, 
			cbv_srv_uav_heap_.GetDescriptorHeapObject(), imgui_font_srv_descriptor_.cpu_handle_, 
			imgui_font_srv_descriptor_.gpu_handle_);

	}

	void GraphicsLibrary::ImguiShutdown()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void GraphicsLibrary::ImguiFrameBegin()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_::ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(300.0f, 500.0f), ImGuiCond_::ImGuiCond_Once);

		ImGui::Begin("Imgui");
	}


	void GraphicsLibrary::ImguiFrameEnd()
	{
		ImGui::End();
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), graphics_command_list_[back_buffer_index_].GetCommandList());
	}
}
