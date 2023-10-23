#pragma once

#include <d3d12.h>
#include <wrl.h>

namespace argent::graphics
{
	class RaytracingPipelineState
	{
	public:
		RaytracingPipelineState();
		~RaytracingPipelineState() = default;

		RaytracingPipelineState(const RaytracingPipelineState&) = delete;
		RaytracingPipelineState(const RaytracingPipelineState&&) = delete;
		RaytracingPipelineState& operator=(const RaytracingPipelineState&) = delete;
		RaytracingPipelineState& operator=(const RaytracingPipelineState&&) = delete;




		ID3D12StateObject* GetStateObject() const { return state_object_.Get(); }
		ID3D12StateObjectProperties* GetStateObjectProperties() const { return state_object_properties_.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12StateObject> state_object_;
		Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> state_object_properties_;
	};
}