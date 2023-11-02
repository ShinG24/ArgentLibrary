#include "Subsystem/Graphics/RenderingManager.h"

#include "Subsystem/Graphics/API/D3D12/ConstantBuffer.h"

#include "Subsystem/Graphics/Common/GraphicsContext.h"
#include "Subsystem/Graphics/Common/GraphicsCommon.h"
#include "Subsystem/Graphics/Common/RenderContext.h"

#include "Subsystem/Graphics/Raytracer.h"

#include "Subsystem/Graphics/GraphicsLibrary.h"

#include "Core/SubsystemLocator.h"
#include "Core/Engine.h"




namespace argent::graphics
{
	void RenderingManager::Awake()
	{
		auto graphics_context = GetEngine()->GetSubsystemLocator()->GetSubsystem<graphics::GraphicsLibrary>()->GetGraphicsContext();
		scene_constant_buffer_ = std::make_unique<dx12::ConstantBuffer>(
			graphics_context->graphics_device_, sizeof(SceneConstant), 
			kNumBackBuffers, graphics_context->cbv_srv_uav_descriptor_heap_);
		raytracer_ = std::make_unique<Raytracer>();
		raytracer_->Awake(graphics_context);
	}

	void RenderingManager::Shutdown()
	{
		raytracer_->Shutdown();
	}

	void RenderingManager::FrameBegin(const RenderContext* render_context, const SceneConstant& scene_data)
	{
		//シーンデータをアップデート
		scene_data_  = scene_data;
		scene_constant_buffer_->CopyToGpu(&scene_data_, render_context->back_buffer_index_);
	}

	void RenderingManager::FrameEnd()
	{
		GetEngine()->GetSubsystemLocator()->GetSubsystem<graphics::GraphicsLibrary>()->CopyToBackBuffer(raytracer_->GetOutputBuffer());
	}

	void RenderingManager::OnRaytrace(const RenderContext* render_context, const GraphicsContext* graphics_context)
	{
		raytracer_->Update(graphics_context->resource_upload_command_list_, graphics_context->resource_upload_command_queue_);
		raytracer_->OnRender(render_context->graphics_command_list_,
			scene_constant_buffer_->GetGpuVirtualAddress(render_context->back_buffer_index_));
	}
}
