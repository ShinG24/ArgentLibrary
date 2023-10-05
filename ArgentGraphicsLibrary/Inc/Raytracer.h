#pragma once

#include <d3d12.h>
#include "../External/DXC/Inc/dxcapi.h"
#include <wrl.h>
#include <vector>

#include <DirectXMath.h>
#include <string>
using float2 = DirectX::XMFLOAT2;
using float3 = DirectX::XMFLOAT3;
using float4 = DirectX::XMFLOAT4;

struct Library
{
	std::vector<std::wstring> exported_symbols_{};
	std::vector<D3D12_EXPORT_DESC> export_desc_{};
	D3D12_DXIL_LIBRARY_DESC library_desc_{};
};

struct RootSignatureAssociation
{
	ID3D12RootSignature* root_signature_;
	ID3D12RootSignature* root_signature_pointer_;
	std::vector<std::wstring> symbols_;
	std::vector<LPCWSTR> symbol_pointers_;
	D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION association_{};
};

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

		void CreateRayGenSignature(const GraphicsDevice& graphics_device);
		void CreateHitSignature(const GraphicsDevice& graphics_device);
		void CreateMissSignature(const GraphicsDevice& graphics_device);

		void CreateStateObjectAndProperties(const GraphicsDevice& graphics_device);
		void BuildShaderExportList(std::vector<std::wstring> exported_symbols);
	private:
		//Shader
		Microsoft::WRL::ComPtr<IDxcBlob> ray_gen_library_;
		Library ray_gen_library_data_;
		RootSignatureAssociation ray_gen_association_;

		Microsoft::WRL::ComPtr<IDxcBlob> miss_library_;
		Library miss_library_data_;
		RootSignatureAssociation miss_association_;

		Microsoft::WRL::ComPtr<IDxcBlob> hit_library_;
		Library hit_library_data_;
		D3D12_HIT_GROUP_DESC hit_group_desc_;
		RootSignatureAssociation hit_association_;


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

		//Root Signature
		Microsoft::WRL::ComPtr<ID3D12RootSignature> ray_gen_signature_;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> hit_signature_;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> miss_signature_;

		//Pipeline
		Microsoft::WRL::ComPtr<ID3D12StateObject> raytracing_state_object_;
		Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> raytracing_state_object_properties_;

		//dummy local and global root signature
		Microsoft::WRL::ComPtr<ID3D12RootSignature> dummy_global_root_signature_;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> dummy_local_root_signature_;
		struct Vertex
		{
			float3 position_;
			float4 color_;
		};
	};
}