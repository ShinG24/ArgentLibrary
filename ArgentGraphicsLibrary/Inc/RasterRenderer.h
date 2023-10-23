#pragma once

#include <d3d12.h>
#include "../External/DXC/Inc/dxcapi.h"
#include <wrl.h>

#include <DirectXMath.h>

#include <memory>

#include "Texture.h"

namespace argent::graphics
{
	class GraphicsDevice;
	class CommandQueue;
	class DescriptorHeap;

	class RasterRenderer
	{
	public:
		RasterRenderer() = default;
		~RasterRenderer() = default;


		void Awake(const GraphicsDevice& graphics_device, const CommandQueue& command_queue, 
		DescriptorHeap& descriptor_heap);
		void OnRender(ID3D12GraphicsCommandList* command_list);
		void OnGui();
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
			DirectX::XMFLOAT3 position_;
			DirectX::XMFLOAT2 texcoord_;
		};

		float alpha_ = 1.0f;
		Microsoft::WRL::ComPtr<IDxcBlob> vertex_shader_;
		Microsoft::WRL::ComPtr<IDxcBlob> pixel_shader_;
		Microsoft::WRL::ComPtr<IDxcBlob> fullscreen_quad_vs_;
		Microsoft::WRL::ComPtr<IDxcBlob> fullscreen_quad_ps_;
		Microsoft::WRL::ComPtr<ID3D12Resource> vertex_buffer_;
		D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state_;


		std::unique_ptr<Texture> texture_;
	};
}