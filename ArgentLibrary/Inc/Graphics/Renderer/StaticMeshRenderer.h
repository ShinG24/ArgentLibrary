#pragma once

#include <DirectXMath.h>

#include <memory>

namespace argent::graphics::dx12
{
	class ConstantBuffer;
}

namespace argent::graphics
{
	struct GraphicsContext;
	class Model;

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

		void Render(const DirectX::XMFLOAT4X4& world_matrix);

	private:
		std::shared_ptr<Model> model_;
		std::unique_ptr<dx12::ConstantBuffer> object_constant_buffer_;
	};
}

