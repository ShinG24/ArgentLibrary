#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>

#include <memory>
#include <vector>
#include <string>

#include "../External/DXC/Inc/dxcapi.h"
#include "../ShaderBindingTableGenerator.h"
#include "../External/Imgui/imgui.h"

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


		void BuildGeometry(const GraphicsDevice& graphics_device);
		void CreateAS(const GraphicsDevice& graphics_device, 
		GraphicsCommandList& command_list, CommandQueue& command_queue);

		void CreatePipeline(const GraphicsDevice& graphics_device);

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

		Microsoft::WRL::ComPtr<ID3D12Resource> raygen_shader_table_;
		Microsoft::WRL::ComPtr<ID3D12Resource> miss_shader_table_;
		Microsoft::WRL::ComPtr<ID3D12Resource> hit_shader_table_;

		enum GeometryType
		{
			Polygon,
			Plane,
			Cube,
			SphereAABB,
			GeometryTypeCount,
		};

		struct Vertex
		{
			float3 position_;
			float3 normal_;
			float2 texcoord_;
		};

		std::unique_ptr<VertexBuffer> vertex_buffers_[GeometryTypeCount];
		std::unique_ptr<IndexBuffer> index_buffers_[GeometryTypeCount];

		Descriptor cube_vertex_descriptor_;
		Descriptor cube_index_descriptor_;

		//Bottom Level
		std::unique_ptr<dxr::BottomLevelAccelerationStructure> blas_[GeometryTypeCount];		
		dxr::TopLevelAccelerationStructure top_level_acceleration_structure_;

		struct Transform
		{
			DirectX::XMFLOAT3 position_{};
			DirectX::XMFLOAT3 scaling_{ 1.0f, 1.0f, 1.0f };
			DirectX::XMFLOAT3 rotation_{};

			DirectX::XMMATRIX CalcWorldMatrix() const
			{
				const auto S = DirectX::XMMatrixScaling(scaling_.x, scaling_.y, scaling_.z);
				const auto R = DirectX::XMMatrixRotationRollPitchYaw(rotation_.x, rotation_.y, rotation_.z);
				const auto T = DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);
				return S * R * T;
			}

			void OnGui()
			{
				if (ImGui::TreeNode("Transform"))
				{
					ImGui::DragFloat3("Position", &position_.x, 0.01f, -FLT_MAX, FLT_MAX);
					ImGui::DragFloat3("Scaling", &scaling_.x, 0.01f, -FLT_MAX, FLT_MAX);
					ImGui::DragFloat3("Rotation", &rotation_.x, 3.14f / 180.0f * 0.1f, -FLT_MAX, FLT_MAX);
					ImGui::TreePop();
				}
			}
		};

		std::string name[GeometryTypeCount]
		{
			"Polygon",
			"Plane",
			"Cube",
			"SphereAABB"
		};

		struct ObjectConstant
		{
			DirectX::XMFLOAT4X4 world_;
			DirectX::XMFLOAT4X4 inv_world_;
		};

		struct Material
		{
			float4 albedo_color_ = float4(1, 1, 1, 1);
			float diffuse_coefficient_ = 0.8f;
			float specular_coefficient_ = 0.2f;
			float reflectance_coefficient_ = 0.2f;
			float specular_power_ = 50.0f;

			void OnGui()
			{
				if (ImGui::TreeNode("Material"))
				{
					ImGui::ColorPicker3("Color", &albedo_color_.x);
					ImGui::DragFloat("Diffuse Coef", &diffuse_coefficient_, 0.001f, 0.0f, 1.0f);
					ImGui::DragFloat("Specular Coef", &specular_coefficient_, 0.001f, 0.0f, 1.0f);
					ImGui::DragFloat("Reflectance Coef", &reflectance_coefficient_, 0.001f, 0.0f, 1.0f);
					ImGui::DragFloat("Specular Power", &specular_power_, 0.1f, 0.0f, 100.0f);
					ImGui::TreePop();
				}
			}
		};

		enum RootSignatureBinder
		{
			ObjectWorld,
			MaterialCB,
			VertexBufferGpuDescriptorHandle,
			RootSignatureBinderCount,
		};
		struct RootSignatureArgument
		{
			Material material_cb_;
			UINT instance_index_;
			D3D12_GPU_DESCRIPTOR_HANDLE vertex_buffer_handle_;
		};

		Transform transforms_[GeometryType::GeometryTypeCount];
		uint8_t* world_mat_map_;
		Microsoft::WRL::ComPtr<ID3D12Resource> world_matrix_buffer_;

		Descriptor object_descriptor_;

		uint8_t* material_map_;
		Microsoft::WRL::ComPtr<ID3D12Resource> material_buffer_;
		Material materials_[GeometryTypeCount];

		UINT hit_shader_table_size_;
		UINT hit_shader_table_stride_;
	};
}