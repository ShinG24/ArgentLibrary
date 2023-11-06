#pragma once

#include <d3d12.h>
#include <DirectXMath.h>

#include <memory>

namespace argent::graphics::dx12
{
	class RootSignature;
	class GraphicsPipelineState;
	class GraphicsCommandList;
	class VertexBuffer;
}

namespace argent::graphics
{
	class Shader;
	class Sprite;

	class SpriteRenderer
	{
	private:

		struct Vertex
		{
			DirectX::XMFLOAT3 position_;
			DirectX::XMFLOAT2 texcoord_;
		};

	public:

		SpriteRenderer();
		~SpriteRenderer() = default;

		SpriteRenderer(const SpriteRenderer&) = delete;
		SpriteRenderer(const SpriteRenderer&&) = delete;
		SpriteRenderer& operator=(const SpriteRenderer&) = delete;
		SpriteRenderer& operator=(const SpriteRenderer&&) = delete;

		void Render(const DirectX::XMFLOAT2& position, const D3D12_VIEWPORT& viewport,
			const dx12::GraphicsCommandList* graphics_command_list);

	private:

		std::shared_ptr<dx12::GraphicsPipelineState> pipeline_state_;
		std::shared_ptr<Shader> vertex_shader_;
		std::shared_ptr<Shader> pixel_shader_;
		std::unique_ptr<dx12::VertexBuffer> vertex_buffer_;

		std::unique_ptr<Sprite> sprite_;

		Vertex vertices_[4];
	};
}

