#include "../Inc/ArgentGraphicsLibrary.h"

#include <windows.h>

#include "../External/d3dx12.h"

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
		GetWindowRect(hwnd_, &rect);
		viewport_ = D3D12_VIEWPORT(0.0f, 0.0f, static_cast<FLOAT>(rect.right - rect.left), 
			static_cast<FLOAT>(rect.bottom - rect.top), 0.0f, 1.0f);
		scissor_rect_ = D3D12_RECT(rect);

		raster_renderer_.Awake(graphics_device_);

		resource_upload_command_list_.Activate();
		raytracer_.Awake(graphics_device_, resource_upload_command_list_,
			resource_upload_queue_, fence_, swap_chain_.GetWidth(), swap_chain_.GetHeight(),
			cbv_srv_uav_heap_);
	}

	void GraphicsLibrary::Shutdown()
	{
		INT last_back_buffer_index = static_cast<INT>(back_buffer_index_) - 1;
		if(last_back_buffer_index < 0) { last_back_buffer_index = kNumBackBuffers; }
		fence_.WaitForGpu(last_back_buffer_index);
	}

	void GraphicsLibrary::FrameBegin()
	{
		HRESULT hr = graphics_device_.GetLatestDevice()->GetDeviceRemovedReason();
		if(FAILED(hr))
		{
			_ASSERT_EXPR(FALSE, L"D3D12Device removed!!");
		}

		auto& command_list = graphics_command_list_[back_buffer_index_];
		command_list.Activate();

		frame_resources_[back_buffer_index_].Activate(command_list);

		command_list.GetCommandList()->RSSetViewports(1u, &viewport_);
		command_list.GetCommandList()->RSSetScissorRects(1u, &scissor_rect_);

		std::vector<ID3D12DescriptorHeap*> heaps = { cbv_srv_uav_heap_.GetDescriptorHeapObject() };
		command_list.GetCommandList()->SetDescriptorHeaps(1u, heaps.data());

		OnRender();
	}

	void GraphicsLibrary::FrameEnd()
	{
		auto& command_list = graphics_command_list_[back_buffer_index_];

		frame_resources_[back_buffer_index_].Deactivate(command_list);

		command_list.Deactivate();

		ID3D12CommandList* command_lists[]
		{
			command_list.GetCommandList()
		};
		main_rendering_queue_.Execute(1u, command_lists);

		main_rendering_queue_.GetCommandQueue()->Signal(fence_.GetFence(), ++fence_value_);

		if(fence_.GetFence()->GetCompletedValue() < fence_value_)
		{
			HANDLE event_handler{};
			fence_.GetFence()->SetEventOnCompletion(fence_value_, event_handler);
			WaitForSingleObject(event_handler, INFINITE);
		}
		swap_chain_.Present();
		back_buffer_index_ = swap_chain_.GetCurrentBackBufferIndex();

		//fence_.PutUpFence(main_rendering_queue_);
		////	main_rendering_queue_.Signal(fence_);

		//fence_.WaitForGpuInCurrentFrame();
		//swap_chain_.Present();

		//back_buffer_index_ = swap_chain_.GetCurrentBackBufferIndex();

	//	fence_.WaitForGpu(back_buffer_index_);
	}

	void GraphicsLibrary::OnRender()
	{
		const auto command_list = graphics_command_list_[back_buffer_index_].GetCommandList();
		if(on_raster_mode_)
		{
			raster_renderer_.OnRender(command_list);
		}
		else
		{
			//raster_renderer_.OnRender(command_list);
			raytracer_.OnRender(graphics_command_list_[back_buffer_index_]);


			//return;
			D3D12_RESOURCE_BARRIER resource_barrier{};
			resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
			resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			resource_barrier.Transition.pResource = frame_resources_[back_buffer_index_].GetBackBuffer();
			command_list->ResourceBarrier(1u, &resource_barrier);

			command_list->CopyResource(frame_resources_[back_buffer_index_].GetBackBuffer(), 
				raytracer_.GetOutputBuffer());

			resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			command_list->ResourceBarrier(1u, &resource_barrier);
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

		fence_.Awake(graphics_device_);


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


	void GraphicsLibrary::CreateRaytracingRootSignature()
	{
		////Global Root Signature
		////Shared across all raytracing shaders invoked during a DispatchRays() call.
		//{
		//	D3D12_DESCRIPTOR_RANGE uav_descriptor_range;
		//	uav_descriptor_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		//	uav_descriptor_range.NumDescriptors = 1;
		//	uav_descriptor_range.BaseShaderRegister = 0;
		//	uav_descriptor_range.RegisterSpace = 0;
		//	uav_descriptor_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//	D3D12_ROOT_PARAMETER root_parameters[2];
		//	root_parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		//	root_parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		//	root_parameters[0].DescriptorTable.NumDescriptorRanges = 1;
		//	root_parameters[0].DescriptorTable.pDescriptorRanges = &uav_descriptor_range;

		//	root_parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		//	root_parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		//	root_parameters[1].Descriptor.ShaderRegister = 0u;
		//	root_parameters[1].Descriptor.RegisterSpace = 0u;

		//	CD3DX12_ROOT_SIGNATURE_DESC global_root_signature_desc(
		//		ARRAYSIZE(root_parameters), root_parameters);

		//	graphics_device_.SerializeAndCreateRootSignature(global_root_signature_desc,
		//		raytracing_global_root_signature_.ReleaseAndGetAddressOf());
		//}

		////Local Root Signature
		////enables a shader to have unique arguments that come from shader table
		//{
		//	D3D12_ROOT_PARAMETER root_parameters[1];
		//	root_parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		//	root_parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		//	root_parameters[0].Constants.Num32BitValues = 8u;
		//	root_parameters[0].Constants.RegisterSpace = 0;
		//	root_parameters[0].Constants.ShaderRegister = 0;

		//	D3D12_ROOT_SIGNATURE_DESC root_signature_desc{};
		//	root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
		//	root_signature_desc.NumParameters = 1;
		//	root_signature_desc.pParameters = root_parameters;
		//	graphics_device_.SerializeAndCreateRootSignature(root_signature_desc, 
		//		raytracing_local_root_signature_.ReleaseAndGetAddressOf());
		//}
	}

}
