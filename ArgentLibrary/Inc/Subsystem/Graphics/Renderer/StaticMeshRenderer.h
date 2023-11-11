#pragma once

#include <DirectXMath.h>

#include <memory>
#include <vector>

namespace argent::graphics::dx12
{
	class ConstantBuffer;
	class GraphicsPipelineState;
}

namespace argent::graphics
{
	struct GraphicsContext;
	struct RenderContext;
	class Mesh;
	class Material;
	class Model;
	class Shader;

	/**
	 * \brief アニメーションのないMesh描画コンポーネント
	 *
	 */
	class StaticMeshRenderer
	{
	private:

		struct ObjectConstant
		{
			DirectX::XMFLOAT4X4 world_;
		};

	public:

		StaticMeshRenderer() =  default;
		~StaticMeshRenderer() = default;

		StaticMeshRenderer(StaticMeshRenderer&) = delete;
		StaticMeshRenderer(StaticMeshRenderer&&) = delete;
		StaticMeshRenderer& operator=(StaticMeshRenderer&) = delete;
		StaticMeshRenderer& operator=(StaticMeshRenderer&&) = delete;

		void Awake(const GraphicsContext* graphics_context, std::shared_ptr<Model> model);

		void Render(const RenderContext* render_context, const DirectX::XMFLOAT4X4& world_matrix);

	private:

		std::vector<std::shared_ptr<Mesh>> meshes_;
		std::vector<std::shared_ptr<Material>> materials_;
		std::shared_ptr<dx12::GraphicsPipelineState> pipeline_state_;

		std::unique_ptr<dx12::ConstantBuffer> object_constant_buffer_;

		std::shared_ptr<Shader> vertex_shader_;
		std::shared_ptr<Shader> pixel_shader_;

	};
}

