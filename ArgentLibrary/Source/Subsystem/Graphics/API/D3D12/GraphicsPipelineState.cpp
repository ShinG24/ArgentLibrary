#include "Subsystem/Graphics/API/D3D12/GraphicsPipelineState.h"


namespace argent::graphics
{
	dx12::GraphicsPipelineState::GraphicsPipelineState(ID3D12Device* device,
		const GraphicsPipelineDesc& pipeline_desc)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC state_desc{};

		state_desc.NodeMask = 0u;
		state_desc.SampleDesc.Count = 1u;
		state_desc.SampleDesc.Quality = 0u;
		state_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		const HRESULT hr = device->CreateGraphicsPipelineState(&state_desc, 
			IID_PPV_ARGS(pipeline_state_object_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Graphics Pipeline State");
	}
}
