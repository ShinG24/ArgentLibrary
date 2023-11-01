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
#include "Wrapper/DXR/AccelerationStructureManager.h"
#include "API/D3D12/ShaderBindingTable.h"
#include "API/D3D12/RootSignature.h"
#include "API/D3D12/RaytracingPipelineState.h"

#include "Resource/Texture.h"
#include "Resource/Model.h"

#include "Component/Transform.h"

using namespace DirectX;

using float2 = DirectX::XMFLOAT2;
using float3 = DirectX::XMFLOAT3;
using float4 = DirectX::XMFLOAT4;

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
		//外に出せるものまとめ
	public:
		static constexpr UINT kNoModelGeometryCounts = 2u;
		enum GeometryType
		{
			Plane,
			Sphere,
			CoralRock,
			GeometryTypeCount,
		};

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

		std::unique_ptr<dx12::VertexBuffer> vertex_buffers_[kNoModelGeometryCounts];
		std::unique_ptr<dx12::IndexBuffer> index_buffers_[kNoModelGeometryCounts];

		struct ObjectConstant
		{
			DirectX::XMFLOAT4X4 world_;
			DirectX::XMFLOAT4X4 inv_world_;
		};

		//Shader
		Microsoft::WRL::ComPtr<IDxcBlob> ray_gen_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> miss_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> plane_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> static_mesh_library_;

		Microsoft::WRL::ComPtr<IDxcBlob> sphere_intersection_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> sphere_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> sphere1_library_;

		argent::component::Transform transforms_[GeometryType::GeometryTypeCount];
		uint8_t* world_mat_map_;
		Microsoft::WRL::ComPtr<ID3D12Resource> world_matrix_buffer_;

		dx12::Descriptor object_descriptor_;

		std::unique_ptr<Texture> skymaps_;

		bool is_wait_ = false;


		//ここら下は必要な機能
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
		
		enum RootSignatureBinder
		{
			ObjectCbv,		//CBV
			MaterialCbv,	//CBV
			TextureStart,		//SRV
			VertexBufferGpuDescriptorHandle,	//SRV
			RootSignatureBinderCount,
		};

		UINT64 width_;
		UINT height_;

		//Output buffer
		Microsoft::WRL::ComPtr<ID3D12Resource> output_buffer_;
		dx12::Descriptor output_descriptor_;

		dx12::AccelerationStructureManager as_manager_;
		UINT tlas_unique_id_[GeometryTypeCount];

	private:

		//For Shader Binding Table
		dx12::ShaderBindingTable raygen_shader_table_;
		dx12::ShaderBindingTable miss_shader_table_;
		dx12::ShaderBindingTable hit_group_shader_table_;

		dx12::RaytracingPipelineState pipeline_state_;

		std::shared_ptr<Model> graphics_model_;

		dx12::RootSignature global_root_signature_;
		dx12::RootSignature raygen_miss_root_signature_;
		dx12::RootSignature hit_group_root_signature_;

	};
}