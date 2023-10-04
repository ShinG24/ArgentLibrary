#pragma once

#include <d3d12.h>
#include "../External/DXC/Inc/dxcapi.h"
#include <wrl.h>


#include <DirectXMath.h>
using float2 = DirectX::XMFLOAT2;
using float3 = DirectX::XMFLOAT3;
using float4 = DirectX::XMFLOAT4;

namespace argent::graphics
{
	class GraphicsDevice;
	class GraphicsCommandList;
	class CommandQueue;
	class Fence;

	class Raytracer
	{
	public:
		Raytracer() = default;
		~Raytracer() = default;

		Raytracer(Raytracer&) = delete;
		Raytracer(Raytracer&&) = delete;
		Raytracer& operator=(Raytracer&) = delete;
		Raytracer& operator=(Raytracer&&) = delete;

		void Awake(const GraphicsDevice& graphics_device, 
			GraphicsCommandList& command_list, const CommandQueue& command_queue,
			Fence& fence);

	private:

		void CreateAS(const GraphicsDevice& graphics_device, 
		GraphicsCommandList& command_list, const CommandQueue& command_queue, 
		Fence& fence);
		void CreateBLAS(const GraphicsDevice& graphics_device, ID3D12GraphicsCommandList4* command_list);
		void CreateTLAS(const GraphicsDevice& graphics_device, ID3D12GraphicsCommandList4* command_list);

		void CreatePipeline(const GraphicsDevice& graphics_device);
	private:
		//Raytracing Object
		Microsoft::WRL::ComPtr<ID3D12RootSignature> raytracing_global_root_signature_;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> raytracing_local_root_signature_;

		//Shader
		Microsoft::WRL::ComPtr<IDxcBlob> ray_gen_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> miss_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> hit_library_;

		//Vertex
		Microsoft::WRL::ComPtr<ID3D12Resource> vertex_buffer_;
		D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;

		//Bottom level AS
		Microsoft::WRL::ComPtr<ID3D12Resource> blas_scratch_buffer_;
		Microsoft::WRL::ComPtr<ID3D12Resource> blas_result_buffer_;

		//Top lvel AS
		Microsoft::WRL::ComPtr<ID3D12Resource> tlas_scratch_buffer_;
		Microsoft::WRL::ComPtr<ID3D12Resource> tlas_result_buffer_;
		Microsoft::WRL::ComPtr<ID3D12Resource> tlas_instance_buffer_;

		struct Vertex
		{
			float3 position_;
			float4 color_;
		};
	};
}