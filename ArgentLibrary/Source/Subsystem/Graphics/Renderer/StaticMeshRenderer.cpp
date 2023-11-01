#include "Subsystem/Graphics/Renderer/StaticMeshRenderer.h"

#include "Subsystem/Graphics/API/D3D12/ConstantBuffer.h"
#include "Subsystem/Graphics/Common/GraphicsCommon.h"
#include "Subsystem/Graphics/Common/GraphicsContext.h"
#include "Subsystem/Graphics/Resource/Model.h"

namespace argent::graphics
{
	void StaticMeshRenderer::Awake(const GraphicsContext* graphics_context, std::shared_ptr<Model> model)
	{
		//TODO �����ŌĂԂ̂͋C����������
		model_->Awake(graphics_context);

		object_constant_buffer_ = std::make_unique<dx12::ConstantBuffer>(graphics_context->graphics_device_, sizeof(ObjectConstant), kNumBackBuffers);
	}

	void StaticMeshRenderer::Render(const DirectX::XMFLOAT4X4& world_matrix)
	{
		ObjectConstant data;
		data.world_ = world_matrix;
		object_constant_buffer_->CopyToGpu(&data, 0u);
	}

}