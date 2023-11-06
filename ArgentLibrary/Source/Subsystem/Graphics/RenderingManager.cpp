#include "Subsystem/Graphics/RenderingManager.h"

#include <imgui.h>


#include "Subsystem/Graphics/API/D3D12/ConstantBuffer.h"

#include "Subsystem/Graphics/Common/GraphicsContext.h"
#include "Subsystem/Graphics/Common/GraphicsCommon.h"
#include "Subsystem/Graphics/Common/RenderContext.h"

#include "Subsystem/Graphics/Raytracer.h"

#include "Subsystem/Graphics/GraphicsLibrary.h"

#include "Core/SubsystemLocator.h"
#include "Core/Engine.h"
#include "Subsystem/Graphics/API/D3D12/GraphicsCommandList.h"
#include "Subsystem/Graphics/API/D3D12/GraphicsPipelineState.h"
#include "Subsystem/Graphics/Resource/Shader.h"


#include "Subsystem/Graphics/Renderer/StaticMeshRenderer.h"

#include "Subsystem/Graphics/Loader/FbxLoader.h"

namespace argent::graphics
{
	std::unique_ptr<StaticMeshRenderer> renderer;
	void RenderingManager::Awake()
	{
		auto graphics_context = GetEngine()->GetSubsystemLocator()->GetSubsystem<graphics::GraphicsLibrary>()->GetGraphicsContext();
		scene_constant_buffer_ = std::make_unique<dx12::ConstantBuffer>(
			graphics_context->graphics_device_, sizeof(SceneConstant), 
			kNumBackBuffers, graphics_context->cbv_srv_uav_descriptor_heap_);

		on_raytrace_ = false;
		if(IsRaytracing())
		{
			raytracer_ = std::make_unique<Raytracer>();
			raytracer_->Awake(graphics_context);
		}

		scene_constant_binding_signature_ = std::make_unique<dx12::RootSignature>();

		//Scene Constant
		scene_constant_binding_signature_->AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV,
			0u, 0u, 1u);
		scene_constant_binding_signature_->AddHeapRangeParameter(1u,
			1u, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0u);
		scene_constant_binding_signature_->AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, 2u, 0u, 61);
		scene_constant_binding_signature_->Create(graphics_context->graphics_device_, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);


		std::shared_ptr<Model> model = LoadFbxFromFile("./Assets/Model/Plantune.fbx");
		renderer = std::make_unique<StaticMeshRenderer>();
		renderer->Awake(graphics_context, model);
	}

	void RenderingManager::Shutdown()
	{
		if(IsRaytracing())
			raytracer_->Shutdown();
	}

	void RenderingManager::OnGui()
	{

	}

	void RenderingManager::FrameBegin(const RenderContext* render_context, const SceneConstant& scene_data)
	{
		//シーンデータをアップデート
		scene_data_  = scene_data;
		scene_constant_buffer_->CopyToGpu(&scene_data_, render_context->back_buffer_index_);

		//シーンコンスタントを設定
		render_context->graphics_command_list_->GetCommandList()->SetGraphicsRootSignature(
			scene_constant_binding_signature_->GetRootSignatureObject());

		render_context->graphics_command_list_->GetCommandList()->SetGraphicsRootConstantBufferView(0u, scene_constant_buffer_->GetGpuVirtualAddress(render_context->back_buffer_index_));
		//render_context->graphics_command_list_->GetCommandList()->SetGraphicsRootDescriptorTable(0u, scene_constant_buffer_->GetDescriptor(
		//	render_context->back_buffer_index_).gpu_handle_);

		DirectX::XMFLOAT4X4 world;
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f);
		DirectX::XMStoreFloat4x4(&world, S);
		renderer->Render(render_context, world);
	}

	void RenderingManager::FrameEnd() const
	{
		if(on_raytrace_)
			GetEngine()->GetSubsystemLocator()->GetSubsystem<graphics::GraphicsLibrary>()->CopyToBackBuffer(raytracer_->GetOutputBuffer());
	}

	void RenderingManager::OnRaytrace(const RenderContext* render_context, const GraphicsContext* graphics_context) const
	{
		raytracer_->Update(graphics_context->resource_upload_command_list_, graphics_context->resource_upload_command_queue_);
		raytracer_->OnRender(render_context->graphics_command_list_,
			scene_constant_buffer_->GetGpuVirtualAddress(render_context->back_buffer_index_));
	}
}
