#pragma once

namespace argent::graphics::dx12
{
	class GraphicsCommandList;
}

namespace argent::graphics
{
	//TODO In Progress
	struct RenderContext
	{
		dx12::GraphicsCommandList* graphics_command_list_;
		int back_buffer_index_;
	};
}