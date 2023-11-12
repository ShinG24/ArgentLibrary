#pragma once

#include <memory>

namespace argent::graphics::dx12
{
	class GraphicsPipelineState;
	class GraphicsCommandList;
}


namespace argent::graphics
{
	class RenderContext;

	/**
	 * \brief RenderPath 
	 */
	class RenderPath
	{
	public:

		RenderPath() = default;
		virtual ~RenderPath() = default;

		RenderPath(const RenderPath&) = delete;
		RenderPath(const RenderPath&&) = delete;
		RenderPath& operator=(const RenderPath&) = delete;
		RenderPath& operator=(const RenderPath&&) = delete;

		virtual void Render(const RenderContext* context) = 0;

	};
}

