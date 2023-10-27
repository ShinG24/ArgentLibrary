#include "../Inc/StaticMeshRenderer.h"

namespace argent::graphics
{
	void StaticMeshRenderer::Awake(const GraphicsContext* graphics_context, std::shared_ptr<Model> model)
	{
		//TODO ‚±‚±‚ÅŒÄ‚Ô‚Ì‚Í‹CŽ‚¿ˆ«‚¢‚æ
		model_->Awake(graphics_context);

		object_constant_buffer_ = std::make_unique<dx12::ConstantBuffer<ObjectConstant>>(graphics_context->graphics_device_, kNumBackBuffers);
	}

	void StaticMeshRenderer::Render(const DirectX::XMFLOAT4X4& world_matrix)
	{
		ObjectConstant data;
		data.world_ = world_matrix;
		object_constant_buffer_->CopyToGpu(data, )
	}

}