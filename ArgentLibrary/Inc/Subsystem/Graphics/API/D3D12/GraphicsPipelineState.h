#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <string>

namespace argent::graphics::dx12
{
	struct BlendDesc
	{
		bool blend_enable_;
	};

	struct DepthStencilDesc
	{
		bool depth_enable_;
		bool write_enable_;
		bool stencil_enable_;
	};

	struct RasterizerDesc
	{
		enum CullMode
		{
			CullNone, CullBack, CullFront,
		};

		bool wire_frame_;
	};

	struct GraphicsPipelineDesc
	{
		BlendDesc blend_desc_;
		DepthStencilDesc depth_stencil_desc_;
		RasterizerDesc rasterizer_desc_;
		std::string vs_filepath_;
		std::string ps_filepath_;
		std::string gs_filepath_;
		std::string hs_filepath_;
		std::string ds_filepath_;
	};

	class GraphicsPipelineState
	{
	public:
		GraphicsPipelineState(ID3D12Device* device, const GraphicsPipelineDesc& pipeline_desc);
		~GraphicsPipelineState() = default;

		GraphicsPipelineState(const GraphicsPipelineState&) = delete;
		GraphicsPipelineState(const GraphicsPipelineState&&) = delete;
		GraphicsPipelineState& operator=(const GraphicsPipelineState&) = delete;
		GraphicsPipelineState& operator=(const GraphicsPipelineState&&) = delete;



	private:

		
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state_object_;

	};
}

