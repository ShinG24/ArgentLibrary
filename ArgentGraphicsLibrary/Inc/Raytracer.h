#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>

#include <memory>
#include <vector>
#include <string>

#include "../External/DXC/Inc/dxcapi.h"

#include "../RaytracingPipelineGenerator.h"
#include "../ShaderBindingTableGenerator.h"
#include "../RootSignatureGenerator.h"
#include "../TopLevelASGenerator.h"
#include "../BottomLevelASGenerator.h"

#include "DescriptorHeap.h"

#include "BottomLevelAccelerationStructure.h"
#include "TopLevelAccelerationStructure.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"


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
			GraphicsCommandList& command_list, CommandQueue& command_queue,
			UINT64 width, UINT height, 
			DescriptorHeap& cbv_srv_uav_descriptor_heap);

		void Update(GraphicsCommandList* graphics_command_list, CommandQueue* upload_command_queue);
		void OnRender(const GraphicsCommandList& command_list, D3D12_GPU_VIRTUAL_ADDRESS scene_constant_gpu_handle);

		ID3D12Resource* GetOutputBuffer() const { return output_buffer_.Get(); }
	private:


	private:

		void BuildGeometry(const GraphicsDevice& graphics_device);
		void CreateAS(const GraphicsDevice& graphics_device, 
		GraphicsCommandList& command_list, CommandQueue& command_queue);

		void CreatePipeline(const GraphicsDevice& graphics_device);

		void BuildShaderExportList(std::vector<std::wstring> exported_symbols);

		void CreateOutputBuffer(const GraphicsDevice& graphics_device, UINT64 width, UINT height);
		void CreateShaderResourceHeap(const GraphicsDevice& graphics_device, 
			DescriptorHeap& cbv_srv_uav_descriptor_heap);

		void CreateShaderBindingTable(const GraphicsDevice& graphics_device);
	private:
		//Shader
		Microsoft::WRL::ComPtr<IDxcBlob> ray_gen_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> miss_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> hit_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> hit1_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> hit2_library_;

		Microsoft::WRL::ComPtr<IDxcBlob> sphere_intersection_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> sphere_closest_hit_library_;

		struct Vertex
		{
			float3 position_;
			float3 normal_;
		};

		std::unique_ptr<VertexBuffer> vertex_buffer0_; 
		std::unique_ptr<VertexBuffer> vertex_buffer1_; 
		std::unique_ptr<VertexBuffer> vertex_buffer2_;
		std::unique_ptr<IndexBuffer> index_buffer_;

		Descriptor cube_vertex_descriptor_;
		Descriptor cube_index_descriptor_;

		//Root Signature
		Microsoft::WRL::ComPtr<ID3D12RootSignature> shared_local_root_signature_;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> hit_local_root_signature_;

		//Pipeline
		Microsoft::WRL::ComPtr<ID3D12StateObject> raytracing_state_object_;
		Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> raytracing_state_object_properties_;

		//dummy local and global root signature
		Microsoft::WRL::ComPtr<ID3D12RootSignature> dummy_global_root_signature_;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> dummy_local_root_signature_;

		//Output buffer
		Microsoft::WRL::ComPtr<ID3D12Resource> output_buffer_;
		Descriptor output_descriptor_;
		Descriptor tlas_result_descriptor_;

		nv_helpers_dx12::ShaderBindingTableGenerator sbt_generator_;
		Microsoft::WRL::ComPtr<ID3D12Resource> sbt_storage_;
		
		UINT64 width_;
		UINT height_;

		//In Tutorial
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


		Microsoft::WRL::ComPtr<ID3D12Resource> raygen_shader_table_;
		Microsoft::WRL::ComPtr<ID3D12Resource> miss_shader_table_;
		Microsoft::WRL::ComPtr<ID3D12Resource> hit_shader_table_;

		//Bottom Level
		std::unique_ptr<BottomLevelAccelerationStructure> bottom_level_0_;
		std::unique_ptr<BottomLevelAccelerationStructure> bottom_level_1_;
		std::unique_ptr<BottomLevelAccelerationStructure> bottom_level_2_;

		TopLevelAccelerationStructure top_level_acceleration_structure_;

		UINT64 tlas_result_size_;
		UINT64 tlas_scratch_size_;
		UINT64 tlas_instance_size_;

		struct Transform
		{
			DirectX::XMFLOAT3 position_{};
			DirectX::XMFLOAT3 scaling_{ 1.0f, 1.0f, 1.0f };
			DirectX::XMFLOAT3 rotation_{};
		};

		Transform cube_transform_;

		Descriptor object_descriptor_;
		Microsoft::WRL::ComPtr<ID3D12Resource> object_world_buffer_;
	};
}