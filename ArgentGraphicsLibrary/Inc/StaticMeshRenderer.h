#pragma once

#include <DirectXMath.h>

#include "GraphicsContext.h"

#include "ConstantBuffer.h"

#include "Model.h"

namespace argent::graphics
{
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
		std::unique_ptr<dx12::ConstantBuffer<ObjectConstant>> object_constant_buffer_;
	};
}

