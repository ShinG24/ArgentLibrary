#pragma once

#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>

#include <memory>

#include <dxcapi.h>

#include "GameResource/Texture.h"


namespace argent::graphics::dx12
{
	class GraphicsDevice;
	class CommandQueue;
	class DescriptorHeap;
}
namespace argent::graphics
{

	class RasterRenderer
	{
	public:
		RasterRenderer() = default;
		~RasterRenderer() = default;


		void Awake(const dx12::GraphicsDevice& graphics_device, const dx12::CommandQueue& command_queue, 
			dx12::DescriptorHeap& descriptor_heap);
		void OnRender(ID3D12GraphicsCommandList* command_list);
		void OnGui();
		bool IsInputEnter() const { return input_enter_ && alpha_ < 0.01;}
	private:

		void CreateVertexBuffer(const dx12::GraphicsDevice& graphics_device);
		void CreateRootSignatureAndPipeline(const dx12::GraphicsDevice& graphics_device);

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
		bool input_enter_ = false;

		std::unique_ptr<Texture> texture_;
	};
}