#include "Subsystem/Graphics/Renderer/StaticMeshRenderer.h"

#include "Subsystem/Graphics/API/D3D12/ConstantBuffer.h"
#include "Subsystem/Graphics/API/D3D12/VertexBuffer.h"
#include "Subsystem/Graphics/API/D3D12/IndexBuffer.h"
#include "Subsystem/Graphics/API/D3D12/GraphicsCommandList.h"
#include "Subsystem/Graphics/API/D3D12/GraphicsPipelineState.h"
#include "Subsystem/Graphics/Common/GraphicsCommon.h"
#include "Subsystem/Graphics/Common/GraphicsContext.h"
#include "Subsystem/Graphics/Common/RenderContext.h"
#include "Subsystem/Graphics/Resource/Mesh.h"
#include "Subsystem/Graphics/Resource/Model.h"


#include "Subsystem/Graphics/RenderingManager.h"

#include "Core/SubsystemLocator.h"
#include "Core/Engine.h"
#include "Subsystem/Graphics/Resource/Shader.h"

namespace argent::graphics
{
	void StaticMeshRenderer::Awake(const GraphicsContext* graphics_context, std::shared_ptr<Model> model)
	{
		model_ = model;
		//TODO ‚±‚±‚ÅŒÄ‚Ô‚Ì‚Í‹CŽ‚¿ˆ«‚¢‚æ
		model_->Awake(graphics_context);

		object_constant_buffer_ = std::make_unique<dx12::ConstantBuffer>(graphics_context->graphics_device_, sizeof(ObjectConstant), kNumBackBuffers,
			graphics_context->cbv_srv_uav_descriptor_heap_);

		vertex_shader_ = std::make_shared<Shader>("./Assets/Shader/StaticMesh.vs.hlsl", Shader::Vertex);
		pixel_shader_ = std::make_shared<Shader>("./Assets/Shader/Standard.ps.hlsl", Shader::Pixel);

		dx12::GraphicsPipelineDesc desc{};
		desc.blend_desc_ = dx12::BlendDesc(dx12::BlendDesc::Alpha);
		desc.rasterizer_desc_ = dx12::RasterizerDesc();
		desc.depth_stencil_desc_ = dx12::DepthStencilDesc(true, true, false);
		desc.num_render_targets_ = 1u;
		desc.render_target_view_format_[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.depth_stencil_format_ = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.root_signature_ = GetEngine()->GetSubsystemLocator()->GetSubsystem<RenderingManager>()->GetRasterGlobalRootSignature();
		desc.vertex_shader_ = vertex_shader_;
		desc.pixel_shader_ = pixel_shader_;

		pipeline_state_ = std::make_unique<dx12::GraphicsPipelineState>(graphics_context->graphics_device_->GetDevice(), desc);
	}

	void StaticMeshRenderer::Render(const RenderContext* render_context, const DirectX::XMFLOAT4X4& world_matrix)
	{
		const auto command_list = render_context->graphics_command_list_->GetCommandList();

		pipeline_state_->SetOnCommandList(command_list);
		command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ObjectConstant data;
		data.world_ = world_matrix;
		object_constant_buffer_->CopyToGpu(&data, render_context->back_buffer_index_);

		command_list->SetGraphicsRootDescriptorTable(1u,
			object_constant_buffer_->GetDescriptor(render_context->back_buffer_index_).gpu_handle_);

		for(auto& m : model_->GetMeshes())
		{
			m->SetVertexBuffersAndIndexBuffer(command_list);
			
			command_list->DrawIndexedInstanced(m->GetRenderingIndexCount(), 1u, 0u, 0u, 0u);
		}
	}
}
