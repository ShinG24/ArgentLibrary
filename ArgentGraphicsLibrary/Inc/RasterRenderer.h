#pragma once

#include <d3d12.h>
#include "../External/DXC/Inc/dxcapi.h"
#include <wrl.h>

namespace argent::graphics
{
	class GraphicsDevice;

	class RasterRenderer
	{
	public:
		RasterRenderer() = default;
		~RasterRenderer() = default;


		void Awake(const GraphicsDevice& graphics_device);
		void OnRender(ID3D12GraphicsCommandList* command_list);

	private:

		void CreateVertexBuffer(const GraphicsDevice& graphics_device);
		void CreateRootSignatureAndPipeline(const GraphicsDevice& graphics_device);
	private:


		//TODO
		struct float2
		{
			float x_;
			float y_;
		};
		struct Vertex
		{
			float2 position_;
			float2 texcoord_;
		};
		Microsoft::WRL::ComPtr<IDxcBlob> vertex_shader_;
		Microsoft::WRL::ComPtr<IDxcBlob> pixel_shader_;
		Microsoft::WRL::ComPtr<ID3D12Resource> vertex_buffer_;
		D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state_;
	};
}