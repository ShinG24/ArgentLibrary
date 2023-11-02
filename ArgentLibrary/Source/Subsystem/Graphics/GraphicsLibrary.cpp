#include "Subsystem/Graphics/GraphicsLibrary.h"

#include <windows.h>

#include <d3dx12.h>
#include <imgui.h>


#include "Subsystem/Platform/Platform.h"

#include "Subsystem/Graphics/API/DXGI/SwapChain.h"
#include "Subsystem/Graphics/API/DXGI/DxgiFactory.h"

#include "Subsystem/Graphics/API/D3D12/GraphicsDevice.h"
#include "Subsystem/Graphics/API/D3D12/CommandQueue.h"
#include "Subsystem/Graphics/API/D3D12/GraphicsCommandList.h"
#include "Subsystem/Graphics/API/D3D12/DescriptorHeap.h"
#include "Subsystem/Graphics/API/D3D12/BottomLevelAccelerationStructure.h"
#include "Subsystem/Graphics/API/D3D12/TopLevelAccelerationStructure.h"

#include "Subsystem/Graphics/Wrapper/DXR/AccelerationStructureManager.h"
#include "Subsystem/Graphics/Wrapper/DXR/ShaderLibrary.h"
#include "Subsystem/Graphics/Wrapper/DXR/ShaderLibraryManager.h"
#include "Subsystem/Graphics/Wrapper/ImGuiWrapper.h"
#include "Subsystem/Graphics/Wrapper/FrameResource.h"

#include "Subsystem/Graphics/Common/RenderContext.h"


#include "Core/SubsystemLocator.h"
#include "Core/Engine.h"



#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3D12.lib")

//DirectX12 Agility SDK
extern "C" { __declspec ( dllexport ) extern const UINT D3D12SDKVersion = 610 ;}
extern "C" { __declspec ( dllexport ) extern const char8_t* D3D12SDKPath = u8"./D3D12/"; }


//Dxrをサポートしていなかった場合の挙動について
//0 何もしない
//1 警告を出して落とす
#define _ASSERT_IF_DXR_NOT_SUPPORTED_	0

namespace argent::graphics
{
	GraphicsLibrary::GraphicsLibrary()
	{
		dxgi_factory_ = std::make_unique<DxgiFactory>();
		swap_chain_ = std::make_unique<SwapChain>();

		graphics_device_ = std::make_unique<dx12::GraphicsDevice>();
		main_rendering_queue_ = std::make_unique<dx12::CommandQueue>();
		resource_upload_queue_ = std::make_unique<dx12::CommandQueue>();
		graphics_command_list_[0] = std::make_unique<dx12::GraphicsCommandList>();
		graphics_command_list_[1] = std::make_unique<dx12::GraphicsCommandList>();
		graphics_command_list_[2] = std::make_unique<dx12::GraphicsCommandList>();
		resource_upload_command_list_ = std::make_unique<dx12::GraphicsCommandList>();
		cbv_srv_uav_heap_ = std::make_unique<dx12::DescriptorHeap>();
		rtv_heap_ = std::make_unique<dx12::DescriptorHeap>();
		dsv_heap_ = std::make_unique<dx12::DescriptorHeap>();
		smp_heap_ = std::make_unique<dx12::DescriptorHeap>();

		as_manager_ = std::make_unique<dx12::AccelerationStructureManager>();

		imgui_wrapper_ = std::make_unique<ImGuiWrapper>();

		frame_resources_[0] = std::make_unique<FrameResource>();
		frame_resources_[1] = std::make_unique<FrameResource>();
		frame_resources_[2] = std::make_unique<FrameResource>();
	}

	void GraphicsLibrary::Awake()
	{
		//エンジンから頑張ってウィンドウハンドルを取得
		hwnd_ = GetEngine()->GetSubsystemLocator()->GetSubsystem<platform::Platform>()->GetHwnd();

		//デバッグレイヤーの有効化
		OnDebugLayer();

		dxgi_factory_->Awake();
		graphics_device_->Awake(dxgi_factory_->GetIDxgiFactory());

		//Check Raytracing tier supported
		const bool raytracing_supported = graphics_device_->CheckDxrSupported();

#if _ASSERT_IF_DXR_NOT_SUPPORTED_
		_ASSERT_EXPR(raytracing_supported, L"DXR does not Supported");
#endif

		//デバイス依存のオブジェクトを作成
		CreateDeviceDependencyObjects();

		//Contextにセットできる変数をセットしていく
		graphics_context_.graphics_device_ = graphics_device_.get();
		graphics_context_.resource_upload_command_queue_ = resource_upload_queue_.get();
		graphics_context_.resource_upload_command_list_ = resource_upload_command_list_.get();
		graphics_context_.cbv_srv_uav_descriptor_heap_ = cbv_srv_uav_heap_.get();
		graphics_context_.rtv_descriptor_ = rtv_heap_.get();
		graphics_context_.dsv_descriptor_ = dsv_heap_.get();
		graphics_context_.as_manager_ = as_manager_.get();

		//ウィンドウサイズ依存のオブジェクトを作成
		CreateWindowDependencyObjects();
	}

	void GraphicsLibrary::Shutdown()
	{
		main_rendering_queue_->WaitForGpu();
		resource_upload_queue_->WaitForGpu();
		imgui_wrapper_->Shutdown();
	}

	RenderContext GraphicsLibrary::FrameBegin()
	{
		HRESULT hr = graphics_device_->GetLatestDevice()->GetDeviceRemovedReason();
		if(FAILED(hr))
		{
			_ASSERT_EXPR(FALSE, L"D3D12Device removed!!");
		}

		const auto& command_list = graphics_command_list_[back_buffer_index_];
		command_list->Activate();

		frame_resources_[back_buffer_index_]->Activate(command_list.get());

		command_list->SetViewports(1u, &viewport_);
		command_list->SetRects(1u, &scissor_rect_);

		std::vector heaps = { cbv_srv_uav_heap_->GetDescriptorHeapObject() };
		command_list->GetCommandList()->SetDescriptorHeaps(1u, heaps.data());

		imgui_wrapper_->FrameBegin();


		RenderContext render_context{};
		render_context.graphics_command_list_ = command_list.get();
		render_context.back_buffer_index_ = back_buffer_index_;

		return render_context;
	}

	void GraphicsLibrary::FrameEnd()
	{
		const auto& command_list = graphics_command_list_[back_buffer_index_];
		imgui_wrapper_->FrameEnd(command_list->GetCommandList());

		frame_resources_[back_buffer_index_]->Deactivate(command_list.get());

		command_list->Deactivate();

		ID3D12CommandList* command_lists[]{ command_list->GetCommandList() };
		main_rendering_queue_->Execute(1u, command_lists);

		swap_chain_->Present();

		main_rendering_queue_->Signal(back_buffer_index_);

		back_buffer_index_ = swap_chain_->GetCurrentBackBufferIndex();
		main_rendering_queue_->WaitForGpu(back_buffer_index_);
	}

	void GraphicsLibrary::CopyToBackBuffer(ID3D12Resource* p_resource) const
	{
		auto& command_list = graphics_command_list_[back_buffer_index_];
		//レイトレーシングの結果をコピーする
		command_list->SetTransitionBarrier(frame_resources_[back_buffer_index_]->GetBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, 
			D3D12_RESOURCE_STATE_COPY_DEST);
		command_list->GetCommandList()->CopyResource(
			frame_resources_[back_buffer_index_]->GetBackBuffer(), 
				p_resource);
		command_list->SetTransitionBarrier(frame_resources_[back_buffer_index_]->GetBackBuffer(), 
			D3D12_RESOURCE_STATE_COPY_DEST, 
			D3D12_RESOURCE_STATE_RENDER_TARGET);

	}

	void GraphicsLibrary::CreateDeviceDependencyObjects()
	{
		main_rendering_queue_->Awake(graphics_device_->GetLatestDevice(), L"Main Rendering Queue");
		resource_upload_queue_->Awake(graphics_device_->GetLatestDevice(), L"Resource Upload Queue");

		cbv_srv_uav_heap_->Awake(graphics_device_.get(), dx12::DescriptorHeap::HeapType::CbvSrvUav, 10000);
		rtv_heap_->Awake(graphics_device_.get(), dx12::DescriptorHeap::HeapType::Rtv, 100);
		dsv_heap_->Awake(graphics_device_.get(), dx12::DescriptorHeap::HeapType::Dsv, 100);
		smp_heap_->Awake(graphics_device_.get(), dx12::DescriptorHeap::HeapType::Smp, 50);

		graphics_command_list_[0]->Awake(graphics_device_->GetDevice());
		graphics_command_list_[1]->Awake(graphics_device_->GetDevice());
		graphics_command_list_[2]->Awake(graphics_device_->GetDevice());
		resource_upload_command_list_->Awake(graphics_device_->GetDevice());
							
		imgui_wrapper_->Awake(graphics_device_.get(), cbv_srv_uav_heap_.get(), hwnd_);
	}

	void GraphicsLibrary::CreateWindowDependencyObjects()
	{
		swap_chain_->Awake(hwnd_, dxgi_factory_->GetIDxgiFactory(), main_rendering_queue_->GetCommandQueue(), kNumBackBuffers);
		back_buffer_index_ = swap_chain_->GetCurrentBackBufferIndex();

		graphics_context_.swap_chain_ = swap_chain_.get();


		for (int i = 0; i < kNumBackBuffers; ++i)
		{
			frame_resources_[i]->Awake(&graphics_context_, i);
		}

		RECT rect{};
		GetClientRect(hwnd_, &rect);
		viewport_ = D3D12_VIEWPORT(0.0f, 0.0f, static_cast<FLOAT>(rect.right - rect.left),
			static_cast<FLOAT>(rect.bottom - rect.top), 0.0f, 1.0f);
		scissor_rect_ = D3D12_RECT(rect);
	}

	void GraphicsLibrary::OnDebugLayer() const
	{
#ifdef _DEBUG
		HRESULT hr{ S_OK };
		ID3D12Debug* debugLayer = nullptr;
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Valied D3d12 Debug Layer");

		debugLayer->EnableDebugLayer();
		debugLayer->Release();
#endif
	}
}
