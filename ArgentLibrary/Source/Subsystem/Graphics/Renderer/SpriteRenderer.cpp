#include "Subsystem/Graphics/Renderer/SpriteRenderer.h"

#include "Subsystem/Graphics/API/D3D12/GraphicsCommandList.h"
#include "Subsystem/Graphics/API/D3D12/RootSignature.h"
#include "Subsystem/Graphics/API/D3D12/GraphicsPipelineState.h"
#include "Subsystem/Graphics/API/D3D12/VertexBuffer.h"


#include "Subsystem/Graphics/Resource/Shader.h"
#include "Subsystem/Graphics/Resource/Texture.h"
#include "Subsystem/Graphics/Resource/Sprite.h"

namespace argent::graphics
{
	void SpriteRenderer::Render(const DirectX::XMFLOAT2& position, const D3D12_VIEWPORT& viewport,
		const dx12::GraphicsCommandList* graphics_command_list)
	{
		//auto vertex_func = [&](Vertex dst[4])
		//{
		//	DirectX::XMFLOAT3 ndc_position = DirectX::XMFLOAT3(position.x / viewport.Width, position.y / viewport.Height, 0u);;
		//	dst[0].position_ = 
		//	dst[0].texcoord_ = sprite_->GetUvOrigin();

		//	dst[1].position_ = DirectX::XMFLOAT3(position.x, )
		//};
		////頂点情報のアップデート
		//vertices_[0].position_ = ;


		//vertex_buffer_->CopyToGpu(vertices_);

		//pipeline_state_->SetOnCommandList(graphics_command_list->GetCommandList());
		//graphics_command_list->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		//const UINT bind_data = sprite_->GetTexture()->GetHeapIndex();

		//graphics_command_list->GetCommandList()->SetGraphicsRoot32BitConstant(0u, bind_data, 0u);

		//graphics_command_list->GetCommandList()->DrawInstanced(4u, 1u, 0u, 0u);

	}
}
