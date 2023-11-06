#include "Subsystem/Graphics/API/D3D12/GraphicsPipelineState.h"


#include "Subsystem/Graphics/API/D3D12/RootSignature.h"

#include "Subsystem/Graphics/Resource/Shader.h"

namespace argent::graphics::dx12
{
	DXGI_FORMAT GetDxgiFormat(D3D_REGISTER_COMPONENT_TYPE type, BYTE mask);

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

	RasterizerDesc::RasterizerDesc(D3D12_FILL_MODE fill_mode, D3D12_CULL_MODE cull_mode, bool is_front_ccw) :
		fill_mode_(fill_mode)
	,	cull_mode_(cull_mode)
	,	is_front_ccw_(is_front_ccw)
	{}

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
			dst.DepthEnable = desc.depth_stencil_desc_.depth_enable_;
			dst.StencilEnable = desc.depth_stencil_desc_.stencil_enable_;
			dst.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			dst.DepthWriteMask = desc.depth_stencil_desc_.write_enable_ ? 
				D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		};

		std::vector<D3D12_INPUT_ELEMENT_DESC> element_desc;
		auto input_func = [&](D3D12_INPUT_LAYOUT_DESC& dst)
		{
			D3D12_SHADER_DESC shader_desc{};
			auto reflection_data = desc.vertex_shader_->GetShaderReflection();
			reflection_data->GetDesc(&shader_desc);

			element_desc.resize(shader_desc.InputParameters);
			for(UINT i = 0; i < shader_desc.InputParameters; ++i)
			{
				auto& element = element_desc.at(i);
				D3D12_SIGNATURE_PARAMETER_DESC signature_parameter_desc{};
				reflection_data->GetInputParameterDesc(i, &signature_parameter_desc);

				element.Format = GetDxgiFormat(signature_parameter_desc.ComponentType, 
					signature_parameter_desc.Mask);
				element.SemanticName = signature_parameter_desc.SemanticName;
				element.SemanticIndex = signature_parameter_desc.SemanticIndex;
				element.InputSlot = desc.increment_input_slot_ ? i : 0u;
				element.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
				element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				element.InstanceDataStepRate = 0u;
			}

			dst.pInputElementDescs = element_desc.data();
			dst.NumElements = static_cast<UINT>(element_desc.size());
		};

		auto pipeline_func = [&](D3D12_GRAPHICS_PIPELINE_STATE_DESC& dst)
		{
			dst.pRootSignature = desc.root_signature_->GetRootSignatureObject();

			blend_func(dst.BlendState);
			raster_func(dst.RasterizerState);
			depth_func(dst.DepthStencilState);
			input_func(dst.InputLayout);
			if (desc.vertex_shader_) dst.VS = desc.vertex_shader_->GetShaderByteCode();
			if (desc.pixel_shader_) dst.PS = desc.pixel_shader_->GetShaderByteCode();
			if (desc.geometry_shader_) dst.GS = desc.geometry_shader_->GetShaderByteCode();
			if (desc.domain_shader_) dst.DS = desc.domain_shader_->GetShaderByteCode();
			if (desc.hull_shader_) dst.HS = desc.hull_shader_->GetShaderByteCode();

			dst.NumRenderTargets = desc.num_render_targets_;
			for(UINT i = 0; i < dst.NumRenderTargets; ++i)
			{
				dst.RTVFormats[i] = desc.render_target_view_format_[i];
			}
			dst.DSVFormat = desc.depth_stencil_format_;

			//基本的に共通なパイプラインの設定
			dst.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
			dst.PrimitiveTopologyType = desc.topology_type_;
			dst.NodeMask = 0u;
			dst.SampleDesc.Count = 1u;
			dst.SampleDesc.Quality = 0u;
			dst.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		};

		pipeline_func(state_desc);

		const HRESULT hr = device->CreateGraphicsPipelineState(&state_desc, 
			IID_PPV_ARGS(pipeline_state_object_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Graphics Pipeline State");
	}

	void GraphicsPipelineState::SetOnCommandList(ID3D12GraphicsCommandList* graphics_command_list) const
	{
		graphics_command_list->SetPipelineState(pipeline_state_object_.Get());
	//	graphics_command_list->SetGraphicsRootSignature(root_signature_->GetRootSignatureObject());
	}

	DXGI_FORMAT GetDxgiFormat(D3D_REGISTER_COMPONENT_TYPE type, BYTE mask)
	{
		DXGI_FORMAT ret = DXGI_FORMAT_UNKNOWN;
		if (mask == 0x01) //1
		{
			switch (type)
			{
			case D3D_REGISTER_COMPONENT_FLOAT32:
				ret = DXGI_FORMAT_R32_FLOAT;
				break;
			case D3D_REGISTER_COMPONENT_UINT32:
				ret = DXGI_FORMAT_R32_UINT;
				break;

			}
		}
		else if (mask == 0x03)//2
		{
			switch (type)
			{
			case D3D_REGISTER_COMPONENT_FLOAT32:
				ret = DXGI_FORMAT_R32G32_FLOAT;
				break;
			case D3D_REGISTER_COMPONENT_UINT32:
				ret = DXGI_FORMAT_R32G32_UINT;
				break;
			}
		}
		else if (mask == 0x07)//3
		{
			switch (type)
			{
			case D3D_REGISTER_COMPONENT_FLOAT32:
				ret = DXGI_FORMAT_R32G32B32_FLOAT;
				break;
			case D3D_REGISTER_COMPONENT_UINT32:
				ret = DXGI_FORMAT_R32G32B32_UINT;
				break;
			}
		}
		else if (mask == 0x0f)//4
		{
			switch (type)
			{
			case D3D_REGISTER_COMPONENT_FLOAT32:
				ret = DXGI_FORMAT_R32G32B32A32_FLOAT;
				break;
			case D3D_REGISTER_COMPONENT_UINT32:
				ret = DXGI_FORMAT_R32G32B32A32_UINT;
				break;
			}
		}

		return ret;
	}

}
