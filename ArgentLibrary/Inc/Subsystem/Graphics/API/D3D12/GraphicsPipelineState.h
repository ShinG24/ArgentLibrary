#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <memory>

namespace argent::graphics
{
	class Shader;
}

namespace argent::graphics::dx12
{
	class RootSignature;

	struct BlendDesc
	{
		enum Type
		{
			Alpha,
			Add,
			Max,
		};
		BlendDesc() = default;
		BlendDesc(Type type, bool is_independent_blend_enable = false, bool is_alpha_to_coverage_enable = false);

		bool blend_enable_;
		bool independent_blend_enable_;
		bool alpha_to_coverage_enable_;
		D3D12_RENDER_TARGET_BLEND_DESC rt_blend_desc_[8];
	};

	struct DepthStencilDesc
	{
		bool depth_enable_;
		bool write_enable_;
		bool stencil_enable_;
	};

	struct RasterizerDesc
	{
		RasterizerDesc() = default;
		RasterizerDesc(D3D12_FILL_MODE fill_mode, D3D12_CULL_MODE cull_mode, bool is_front_ccw);

		D3D12_FILL_MODE fill_mode_ = D3D12_FILL_MODE_SOLID;
		D3D12_CULL_MODE cull_mode_ = D3D12_CULL_MODE_NONE;
		bool is_front_ccw_;
		int32_t depth_bias_;
		float depth_bias_clamp_;
		float slope_scaled_depth_bias_;
		bool is_depth_clamp_enable_;
		bool is_multi_sample_enable_;
		bool is_antialiased_line_enable_;
		bool is_conservative_raster_enable_;
	};

	struct GraphicsPipelineDesc
	{
		BlendDesc blend_desc_;
		DepthStencilDesc depth_stencil_desc_;
		RasterizerDesc rasterizer_desc_;
		std::shared_ptr<Shader> vertex_shader_{};
		std::shared_ptr<Shader> pixel_shader_{};
		std::shared_ptr<Shader> geometry_shader_{};
		std::shared_ptr<Shader> hull_shader_{};
		std::shared_ptr<Shader> domain_shader_{};
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topology_type_ = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		std::shared_ptr<RootSignature> root_signature_;
		bool increment_input_slot_;
		UINT num_render_targets_;
		DXGI_FORMAT render_target_view_format_[8];
		DXGI_FORMAT depth_stencil_format_ = DXGI_FORMAT_UNKNOWN;
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

		void SetOnCommandList(ID3D12GraphicsCommandList* graphics_command_list) const;

	private:
		
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state_object_;
		std::shared_ptr<RootSignature> root_signature_;

	};
}

