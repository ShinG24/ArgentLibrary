#include "Subsystem/Graphics/API/D3D12/GraphicsPipelineState.h"


#include "Subsystem/Graphics/Resource/Shader.h"

namespace argent::graphics::dx12
{
	BlendDesc::BlendDesc(Type type, bool is_independent_blend_enable, bool is_alpha_to_coverage_enable) :
		blend_enable_(true)
	,	independent_blend_enable_(is_independent_blend_enable)
	,	alpha_to_coverage_enable_(is_alpha_to_coverage_enable)
	{
		if(type == Alpha)
		{
			rt_blend_desc_[0].BlendEnable = true;
			rt_blend_desc_[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			rt_blend_desc_[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			rt_blend_desc_[0].BlendOp = D3D12_BLEND_OP_ADD;
			rt_blend_desc_[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			rt_blend_desc_[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
			rt_blend_desc_[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
			rt_blend_desc_[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			rt_blend_desc_[0].LogicOpEnable = false;
		}
	}

	GraphicsPipelineState::GraphicsPipelineState(ID3D12Device* device,
	                                             const GraphicsPipelineDesc& desc)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC state_desc{};

		auto blend_func = [&](D3D12_BLEND_DESC& dst)
			{
				dst.IndependentBlendEnable = desc.blend_desc_.alpha_to_coverage_enable_;
				dst.IndependentBlendEnable = desc.blend_desc_.independent_blend_enable_;
				dst.RenderTarget[0] = desc.blend_desc_.rt_blend_desc_[0];
				if (dst.IndependentBlendEnable)
				{
					for (uint32_t i = 1; i < 8; ++i)
					{
						dst.RenderTarget[i] = desc.blend_desc_.rt_blend_desc_[i];
					}
				}
			};

		auto raster_func = [&](D3D12_RASTERIZER_DESC& dst)
			{
				dst.CullMode = desc.rasterizer_desc_.cull_mode_;
				dst.FillMode = desc.rasterizer_desc_.fill_mode_;
				dst.FrontCounterClockwise = desc.rasterizer_desc_.is_front_ccw_;
				dst.DepthBias = desc.rasterizer_desc_.depth_bias_;
				dst.DepthBiasClamp = desc.rasterizer_desc_.depth_bias_clamp_;
				dst.SlopeScaledDepthBias = desc.rasterizer_desc_.slope_scaled_depth_bias_;
				dst.DepthClipEnable = desc.rasterizer_desc_.is_depth_clamp_enable_;
				dst.MultisampleEnable = desc.rasterizer_desc_.is_multi_sample_enable_;
				dst.AntialiasedLineEnable = desc.rasterizer_desc_.is_antialiased_line_enable_;
				dst.ForcedSampleCount = 0u;
				dst.ConservativeRaster = desc.rasterizer_desc_.is_conservative_raster_enable_ ?
					D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
			};

		auto depth_func = [&](D3D12_DEPTH_STENCIL_DESC& dst)
		{
			
		}


		state_desc.PrimitiveTopologyType = desc.topology_type_;


		//シェーダーセット
		state_desc.VS = desc.vertex_shader_->GetShaderByteCode();
		state_desc.PS = desc.pixel_shader_->GetShaderByteCode();
		if (desc.geometry_shader_) state_desc.GS = desc.geometry_shader_->GetShaderByteCode();
		if (desc.domain_shader_) state_desc.DS = desc.domain_shader_->GetShaderByteCode();
		if (desc.hull_shader_) state_desc.HS = desc.hull_shader_->GetShaderByteCode();
		if (desc.geometry_shader_) state_desc.GS = desc.geometry_shader_->GetShaderByteCode();


		//基本的に共通なパイプラインの設定
		state_desc.NodeMask = 0u;
		state_desc.SampleDesc.Count = 1u;
		state_desc.SampleDesc.Quality = 0u;
		state_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		const HRESULT hr = device->CreateGraphicsPipelineState(&state_desc, 
			IID_PPV_ARGS(pipeline_state_object_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Graphics Pipeline State");
	}
}
