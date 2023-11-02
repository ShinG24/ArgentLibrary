#include "Subsystem/Graphics/GraphicsLibrary.h"

#include <windows.h>

#include <d3dx12.h>

//Imgui
#include <imgui.h>

#include "Core/SubsystemLocator.h"
#include "Core/Engine.h"


#include "Subsystem/Graphics/Wrapper/DXR/ShaderLibrary.h"
#include "Subsystem/Graphics/Wrapper/DXR/ShaderLibraryManager.h"


#include "Subsystem/Platform/Platform.h"
#include "Subsystem/Input/InputManager.h"

#include "Subsystem/Timer/Timer.h"
#include "Subsystem/Input/InputManager.h"

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

	}

	void GraphicsLibrary::Awake()
	{
		hwnd_ = GetEngine()->GetSubsystemLocator()->GetSubsystem<platform::Platform>()->GetHwnd();
		

#ifdef _DEBUG
		OnDebugLayer();
#endif
		dxgi_factory_->Awake();
		graphics_device_->Awake(dxgi_factory_->GetIDxgiFactory());

		//Check Raytracing tier supported
		const bool raytracing_supported = graphics_device_->CheckDxrSupported();

#if _ASSERT_IF_DXR_NOT_SUPPORTED_
		_ASSERT_EXPR(raytracing_supported, L"DXR does not Supported");
#endif

		CreateDeviceDependencyObjects();

		graphics_context_.graphics_device_ = graphics_device_.get();
		
		graphics_context_.resource_upload_command_queue_ = resource_upload_queue_.get();
		graphics_context_.resource_upload_command_list_ = resource_upload_command_list_.get();
		graphics_context_.cbv_srv_uav_descriptor_heap_ = cbv_srv_uav_heap_.get();
		graphics_context_.rtv_descriptor_ = rtv_heap_.get();
		graphics_context_.dsv_descriptor_ = dsv_heap_.get();
		graphics_context_.as_manager_ = as_manager_.get();

		CreateWindowDependencyObjects();


		InitializeScene();
	}

	void GraphicsLibrary::Shutdown()
	{
		raytracer_.Shutdown();
		main_rendering_queue_->WaitForGpu();
		resource_upload_queue_->WaitForGpu();
		imgui_wrapper_.Shutdown();
	}

	void GraphicsLibrary::FrameBegin()
	{
		HRESULT hr = graphics_device_->GetLatestDevice()->GetDeviceRemovedReason();
		if(FAILED(hr))
		{
			_ASSERT_EXPR(FALSE, L"D3D12Device removed!!");
		}

		auto& command_list = graphics_command_list_[back_buffer_index_];
		command_list->Activate();

		frame_resources_[back_buffer_index_].Activate(command_list.get());

		command_list->SetViewports(1u, &viewport_);
		command_list->SetRects(1u, &scissor_rect_);

		std::vector heaps = { cbv_srv_uav_heap_->GetDescriptorHeapObject() };
		command_list->GetCommandList()->SetDescriptorHeaps(1u, heaps.data());

		imgui_wrapper_.FrameBegin();

		OnRender();
	}

	void GraphicsLibrary::FrameEnd()
	{
		auto& command_list = graphics_command_list_[back_buffer_index_];
		imgui_wrapper_.FrameEnd(command_list->GetCommandList());

		frame_resources_[back_buffer_index_].Deactivate(command_list.get());

		command_list->Deactivate();

		ID3D12CommandList* command_lists[]{ command_list->GetCommandList() };
		main_rendering_queue_->Execute(1u, command_lists);

		swap_chain_->Present();

		main_rendering_queue_->Signal(back_buffer_index_);

		back_buffer_index_ = swap_chain_->GetCurrentBackBufferIndex();
		main_rendering_queue_->WaitForGpu(back_buffer_index_);
	}

	void GraphicsLibrary::InitializeScene()
	{
		scene_constant_buffer_ = std::make_unique<dx12::ConstantBuffer>(graphics_device_.get(), sizeof(SceneConstant), kNumBackBuffers);

		raster_renderer_.Awake(&graphics_context_);

		resource_upload_command_list_->Activate();
#if _USE_RAY_TRACER_
		if(!on_raster_mode_)
		{
			raytracer_.Awake(&graphics_context_, swap_chain_->GetWidth(), swap_chain_->GetHeight());
		}
#endif
	}

	void GraphicsLibrary::OnRender()
	{
		//Update Constant Buffer
		{
			if(raster_renderer_.IsInputEnter())
			{
				//Camera Controller
				{
					auto input_manager = GetEngine()->GetSubsystemLocator()->GetSubsystem<input::InputManager>();
					auto keyboard = input_manager->GetKeyboard();
					auto mouse = input_manager->GetMouse();
					using namespace argent::input;
					if (mouse->GetButton(argent::input::MouseButton::RButton)/*右クリックの入力*/)
					{
						//カメラの移動
						{
							//方向を算出
							const auto rotation_matrix = DirectX::XMMatrixRotationRollPitchYaw(camera_rotation_.x, camera_rotation_.y, camera_rotation_.z);
							const DirectX::XMVECTOR front = DirectX::XMVector3Normalize(rotation_matrix.r[2]);
							const DirectX::XMVECTOR up = { 0.0f, 1.0f, 0.0f, 0.0f };
							const DirectX::XMVECTOR right = DirectX::XMVector3Normalize(rotation_matrix.r[0]);

							float front_input = 0;
							float right_input = 0;
							float up_input = 0;

							//前後
							if (keyboard->GetKey(W)/* Keyboard W */) front_input += 1.0f;
							if (keyboard->GetKey(S)/* Keyboard S */) front_input -= 1.0f;

							//左右
							if (keyboard->GetKey(A)/* Keyboard A */) right_input -= 1.0f;
							if (keyboard->GetKey(D)/* Keyboard D */) right_input += 1.0f;

							//上下
							if (keyboard->GetKey(Q)/* Keyboard Q */) up_input -= 1.0f;
							if (keyboard->GetKey(E)/* Keyboard E */) up_input += 1.0f;

							using namespace DirectX;

							const DirectX::XMVECTOR move_vector = DirectX::XMVector3Normalize(front * front_input + up * up_input + right * right_input);
							DirectX::XMFLOAT3 p = { camera_position_.x, camera_position_.y, camera_position_.z };
							DirectX::XMStoreFloat3(&p, DirectX::XMLoadFloat3(&p) + move_vector * move_speed_);
							camera_position_ = { p.x, p.y, p.z, 1.0f };
						}

						//カメラの回転
						{
							const float dx = mouse->GetMovedVec().x;	//マウスX軸方向の移動値
							const float dy = mouse->GetMovedVec().y;	//マウスY軸方向の移動値

							const float x_d_angle = dy * rotation_speed_;
							const float y_d_angle = dx * rotation_speed_;

							camera_rotation_.x += x_d_angle;
							camera_rotation_.y += y_d_angle;
						}
					}
				}

				//Draw on ImGui
				{
					if(ImGui::TreeNode("Performance"))
					{
						float delta_time = Timer::Get()->GetDeltaTime();
						int fps = Timer::Get()->GetFps();
						ImGui::InputFloat("Delta Time", &delta_time);
						ImGui::InputInt("FPS", &fps);
						

						ImGui::TreePop();
					}
					
					if (ImGui::TreeNode("Camera"))
					{
						ImGui::DragFloat3("Position", &camera_position_.x, 0.01f, -FLT_MAX, FLT_MAX);
						ImGui::DragFloat3("Rotation", &camera_rotation_.x, 1.0f / 3.14f * 0.01f, -FLT_MAX, FLT_MAX);
						ImGui::DragFloat("Move Speed", &move_speed_, 0.01f, 0.1f, 10.0f);
						ImGui::DragFloat("Rotation Speed", &rotation_speed_, 0.00001f, 0.00001f, 3.14f);
						ImGui::TreePop();
					}
					ImGui::DragFloat3("Light Position", &light_position.x, 0.01f, -FLT_MAX, FLT_MAX);

					//raster_renderer_.OnGui();
				}
			}

			using namespace DirectX;
			//Update camera forward direction by the rotation
			DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(camera_rotation_.x, camera_rotation_.y, camera_rotation_.z);
			DirectX::XMVECTOR F = R.r[2];
			F = DirectX::XMVector3Normalize(F) * 1000.0f;

			DirectX::XMVECTOR Eye = DirectX::XMLoadFloat4(&camera_position_);
			DirectX::XMVECTOR Focus = Eye + F;
			//Focus.m128_f32[2] += 1.0f;
			DirectX::XMVECTOR Up = DirectX::XMVector3Normalize(R.r[1]);
			auto view = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
			auto proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov_angle_), aspect_ratio_, near_z_, far_z_);

			SceneConstant data{};
			data.camera_position_ = camera_position_;
			DirectX::XMStoreFloat4x4(&data.view_projection_, view * proj);
			DirectX::XMStoreFloat4x4(&data.inv_view_projection_, DirectX::XMMatrixInverse(nullptr, view * proj));
			data.light_position_ = light_position;

			scene_constant_buffer_->CopyToGpu(&data, back_buffer_index_);
		}

		const auto& command_list = graphics_command_list_[back_buffer_index_];
		if(on_raster_mode_)
		{
			raster_renderer_.OnRender(command_list->GetCommandList());
		}
		else
		{
#if _USE_RAY_TRACER_

			if(raster_renderer_.IsInputEnter())
			{
			raytracer_.Update(resource_upload_command_list_.get(), resource_upload_queue_.get());
			raytracer_.OnRender(graphics_command_list_[back_buffer_index_].get(), scene_constant_buffer_->GetGpuVirtualAddress(back_buffer_index_));

			command_list->SetTransitionBarrier(frame_resources_[back_buffer_index_].GetBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);

			command_list->GetCommandList()->CopyResource(frame_resources_[back_buffer_index_].GetBackBuffer(), 
				raytracer_.GetOutputBuffer());

			command_list->SetTransitionBarrier(frame_resources_[back_buffer_index_].GetBackBuffer(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
			}
			else
			{
			raster_renderer_.OnRender(command_list->GetCommandList());
				
			}

#endif
		}
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
							
		imgui_wrapper_.Awake(graphics_device_.get(), cbv_srv_uav_heap_.get(), hwnd_);
	}

	void GraphicsLibrary::CreateWindowDependencyObjects()
	{
		swap_chain_->Awake(hwnd_, dxgi_factory_->GetIDxgiFactory(), main_rendering_queue_->GetCommandQueue(), kNumBackBuffers);
		back_buffer_index_ = swap_chain_->GetCurrentBackBufferIndex();

		graphics_context_.swap_chain_ = swap_chain_.get();


		for (int i = 0; i < kNumBackBuffers; ++i)
		{
			frame_resources_[i].Awake(&graphics_context_, i);
		}

		RECT rect{};
		GetClientRect(hwnd_, &rect);
		viewport_ = D3D12_VIEWPORT(0.0f, 0.0f, static_cast<FLOAT>(rect.right - rect.left),
			static_cast<FLOAT>(rect.bottom - rect.top), 0.0f, 1.0f);
		scissor_rect_ = D3D12_RECT(rect);
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
}
