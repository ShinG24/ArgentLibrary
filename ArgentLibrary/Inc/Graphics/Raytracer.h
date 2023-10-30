#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>

#include <memory>
#include <vector>
#include <string>

#include <dxcapi.h>

#include "API/D3D12/DescriptorHeap.h"

#include "API/D3D12/VertexBuffer.h"
#include "API/D3D12/IndexBuffer.h"
#include "API/D3D12/AccelerationStructureManager.h"
#include "API/D3D12/ShaderBindingTable.h"
#include "API/D3D12/RootSignature.h"
#include "API/D3D12/RaytracingPipelineState.h"

#include "GameResource/Texture.h"

#include "GameResource/Model.h"

using namespace DirectX;

using float2 = DirectX::XMFLOAT2;
using float3 = DirectX::XMFLOAT3;
using float4 = DirectX::XMFLOAT4;

struct Transform
{
	DirectX::XMFLOAT3 position_{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 scaling_{ 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 rotation_{};
	INT coordinate_system_index_;
	static constexpr DirectX::XMFLOAT4X4 kCoordinateSystem[4]
	{
		{ -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }, // 0:RHS Y-UP
		{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }, // 1:LHS Y-UP
		{ -1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 }, // 2:RHS Z-UP
		{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 }, // 3:LHS Z-UP

	};

	const std::string kCoordinateSystemStr[4]
	{
		{"RHS Y-UP"},
		{"LHS Y-UP"},
		{"RHS Z-UP"},
		{"LHS Z-UP"},
	};

	DirectX::XMMATRIX CalcWorldMatrix() const
	{
		const auto C = DirectX::XMLoadFloat4x4(&kCoordinateSystem[coordinate_system_index_]);
		const auto S = DirectX::XMMatrixScaling(scaling_.x, scaling_.y, scaling_.z);
		const auto R = DirectX::XMMatrixRotationRollPitchYaw(rotation_.x, rotation_.y, rotation_.z);
		const auto T = DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);
		return C * S * R * T;
	}

	void OnGui();
};

struct Vertex
{
	float3 position_;
	float3 normal_;
	float3 tangent_;
	float3 binormal_;
	float2 texcoord_;
};

namespace argent::graphics::dx12
{
	class GraphicsDevice;
	class GraphicsCommandList;
	class CommandQueue;
}

namespace argent::graphics
{
	struct GraphicsContext;

	class Raytracer
	{
	public:
		static constexpr UINT kNoModelGeometryCounts = 2u;
		enum GeometryType
		{
			Plane,
			Sphere,
			CoralRock,
			GeometryTypeCount,
		};

	private:
		std::string name[GeometryTypeCount]
		{
			"Plane",
			"Sphere",
			"CoralRock",
		};

		std::vector<std::wstring> kHitGroupName/*[GeometryTypeCount]*/
		{
			L"HG_Plane",
			L"HG_Sphere",
			L"HG_CoralRock",
		};

		std::string filepaths[GeometryTypeCount - kNoModelGeometryCounts]
		{
			"./Assets/Model/CoralRock.fbx",
		};


	public:
		Raytracer() = default;
		~Raytracer() = default;

		Raytracer(Raytracer&) = delete;
		Raytracer(Raytracer&&) = delete;
		Raytracer& operator=(Raytracer&) = delete;
		Raytracer& operator=(Raytracer&&) = delete;

		void Awake(const dx12::GraphicsDevice* graphics_device, 
			dx12::GraphicsCommandList* command_list, dx12::CommandQueue* command_queue,
			UINT64 width, UINT height, 
			dx12::DescriptorHeap* cbv_srv_uav_descriptor_heap, const GraphicsContext* graphics_context);
		void Shutdown();

		void Update(dx12::GraphicsCommandList* graphics_command_list, dx12::CommandQueue* upload_command_queue);
		void OnRender(const dx12::GraphicsCommandList* command_list, D3D12_GPU_VIRTUAL_ADDRESS scene_constant_gpu_handle);


		ID3D12Resource* GetOutputBuffer() const { return output_buffer_.Get(); }
	private:

		void BuildGeometry(const dx12::GraphicsDevice* graphics_device);
		void CreateAS(const dx12::GraphicsDevice* graphics_device, 
			dx12::GraphicsCommandList* command_list, dx12::CommandQueue* command_queue);

		void CreatePipeline(const dx12::GraphicsDevice* graphics_device);

		void CreateOutputBuffer(const dx12::GraphicsDevice* graphics_device, UINT64 width, UINT height);
		void CreateShaderResourceHeap(const dx12::GraphicsDevice* graphics_device, 
			dx12::DescriptorHeap* cbv_srv_uav_descriptor_heap);

		void CreateShaderBindingTable(const dx12::GraphicsDevice* graphics_device);

	private:
		//Shader
		Microsoft::WRL::ComPtr<IDxcBlob> ray_gen_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> miss_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> plane_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> static_mesh_library_;

		Microsoft::WRL::ComPtr<IDxcBlob> sphere_intersection_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> sphere_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> sphere1_library_;


		//Output buffer
		Microsoft::WRL::ComPtr<ID3D12Resource> output_buffer_;
		dx12::Descriptor output_descriptor_;
		dx12::Descriptor tlas_result_descriptor_;
		
		UINT64 width_;
		UINT height_;

		std::unique_ptr<dx12::VertexBuffer> vertex_buffers_[kNoModelGeometryCounts];
		std::unique_ptr<dx12::IndexBuffer> index_buffers_[kNoModelGeometryCounts];

		struct ObjectConstant
		{
			DirectX::XMFLOAT4X4 world_;
			DirectX::XMFLOAT4X4 inv_world_;
		};

		enum RootSignatureBinder
		{
			ObjectCbv,		//CBV
			MaterialCbv,	//CBV
			TextureStart,		//SRV
			VertexBufferGpuDescriptorHandle,	//SRV
			RootSignatureBinderCount,
		};

		Transform transforms_[GeometryType::GeometryTypeCount];
		uint8_t* world_mat_map_;
		Microsoft::WRL::ComPtr<ID3D12Resource> world_matrix_buffer_;

		dx12::Descriptor object_descriptor_;

		dx12::AccelerationStructureManager as_manager_;
		UINT tlas_unique_id_[GeometryTypeCount];


		std::unique_ptr<Texture> skymaps_;

		bool is_wait_ = false;


	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> blas_transform_cube_;

		//For Shader Binding Table
		dx12::ShaderBindingTable raygen_shader_table_;
		dx12::ShaderBindingTable miss_shader_table_;
		dx12::ShaderBindingTable hit_group_shader_table_;

		dx12::RaytracingPipelineState pipeline_state_;

		std::shared_ptr<Model> graphics_model_;

		dx12::RootSignature global_root_signature_;
		dx12::RootSignature raygen_miss_root_signature_;
		dx12::RootSignature hit_group_root_signature_;
		UINT vertex_counts_ = 0u;
		UINT index_counts_ = 0u;

	public:
		UINT GetVertexCounts() const { return vertex_counts_; }
		UINT GetIndexCounts() const { return index_counts_; }
	};
}