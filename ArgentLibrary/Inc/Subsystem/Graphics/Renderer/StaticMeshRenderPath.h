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
	class StaticMeshRenderPath
	{
	private:

		struct ObjectConstant
		{
			DirectX::XMFLOAT4X4 world_;
		};

	public:

		StaticMeshRenderPath() =  default;
		~StaticMeshRenderPath() = default;
		

		StaticMeshRenderPath(StaticMeshRenderPath&) = delete;
		StaticMeshRenderPath(StaticMeshRenderPath&&) = delete;
		StaticMeshRenderPath& operator=(StaticMeshRenderPath&) = delete;
		StaticMeshRenderPath& operator=(StaticMeshRenderPath&&) = delete;

		void Awake(const GraphicsContext* graphics_context, std::shared_ptr<Model> model);

		void Render(const RenderContext* render_context, const DirectX::XMFLOAT4X4& world_matrix);

	private:

		std::vector<std::shared_ptr<Mesh>> meshes_{};
		std::vector<std::shared_ptr<Material>> materials_{};

		//TODO マテリアルにシェーダーをもたせたときにはここもstd::vector<>にする
		std::shared_ptr<dx12::GraphicsPipelineState> pipeline_state_{};
		std::unique_ptr<dx12::ConstantBuffer> object_constant_buffer_{};

		//TODO 複数メッシュへの対応
		std::vector<std::unique_ptr<dx12::ConstantBuffer>> mesh_constant_buffers_{};

		std::shared_ptr<Shader> vertex_shader_{};
		std::shared_ptr<Shader> pixel_shader_{};
	};
}

