#pragma once

#include <d3d12.h>
#include "../External/DXC/Inc/dxcapi.h"
#include <wrl.h>
#include <vector>

#include <DirectXMath.h>
#include <string>

#include "../RaytracingPipelineGenerator.h"
#include "../ShaderBindingTableGenerator.h"
#include "../RootSignatureGenerator.h"
#include "../TopLevelASGenerator.h"
#include "../BottomLevelASGenerator.h"

#include "DescriptorHeap.h"

using namespace DirectX;

using float2 = DirectX::XMFLOAT2;
using float3 = DirectX::XMFLOAT3;
using float4 = DirectX::XMFLOAT4;

using Microsoft::WRL::ComPtr;

struct AccelerationStructureBuffers
{
    ComPtr<ID3D12Resource> pScratch;      // Scratch memory for AS builder
    ComPtr<ID3D12Resource> pResult;       // Where the AS is
    ComPtr<ID3D12Resource> pInstanceDesc; // Hold the matrices of the instances
};

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
			Fence& fence, UINT64 width, UINT height, 
			DescriptorHeap& cbv_srv_uav_descriptor_heap);

		void OnRender(const GraphicsCommandList& command_list);

		ID3D12Resource* GetOutputBuffer() const { return output_buffer_.Get(); }
	private:


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

		void CreateOutputBuffer(const GraphicsDevice& graphics_device, UINT64 width, UINT height);
		void CreateShaderResourceHeap(const GraphicsDevice& graphics_device, 
			DescriptorHeap& cbv_srv_uav_descriptor_heap);

		void CreateShaderBindingTable(const GraphicsDevice& graphics_device);
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

		//Output buffer
		Microsoft::WRL::ComPtr<ID3D12Resource> output_buffer_;
		//Descriptor output_descriptor_;
		//Descriptor tlas_result_descriptor_;

		nv_helpers_dx12::ShaderBindingTableGenerator sbt_generator_;
		Microsoft::WRL::ComPtr<ID3D12Resource> sbt_storage_;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_;


		UINT width_;
		UINT height_;

		struct Vertex
		{
			float3 position_;
			float4 color_;
		};




		//In Tutorial
		ComPtr<ID3D12Resource> bottom_level_as_;
		nv_helpers_dx12::TopLevelASGenerator top_level_as_generator_;
		AccelerationStructureBuffers top_level_as_buffer_;
		std::vector<std::pair<ComPtr<ID3D12Resource>, XMMATRIX>> instances_;

		AccelerationStructureBuffers CreateBottomLevelAs(
			const GraphicsDevice& graphics_device,
		ID3D12GraphicsCommandList4* command_list,
		std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vertex_buffers);

		void CreateTopLevelAs(const GraphicsDevice& graphics_device,
		ID3D12GraphicsCommandList4* command_list,
		const std::vector<std::pair<ComPtr<ID3D12Resource>, XMMATRIX>>& instances);


		D3D12_HEAP_PROPERTIES default_heap{D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0u, 0u };
		D3D12_HEAP_PROPERTIES upload_heap{D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0u, 0u };
	};
}