#pragma once

#include <d3d12.h>
#include <wrl.h>

namespace argent::graphics
{
	class GraphicsDevice;
}

//TODO Shader Table Under Development
namespace argent::graphics::dxr
{
	class RaytracingShaderTable
	{
	public:
		RaytracingShaderTable(const GraphicsDevice* graphics_device);
		~RaytracingShaderTable() = default;

		RaytracingShaderTable(const RaytracingShaderTable&) = delete;
		RaytracingShaderTable(const RaytracingShaderTable&&) = delete;
		RaytracingShaderTable operator=(const RaytracingShaderTable&) = delete;
		RaytracingShaderTable operator=(const RaytracingShaderTable&&) = delete;



	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> shader_table_object_;
		UINT table_size_;
		UINT table_stride_;
	};
}