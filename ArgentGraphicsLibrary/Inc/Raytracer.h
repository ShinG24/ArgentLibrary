#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>

#include <memory>
#include <vector>
#include <string>


#include "../External/DXC/Inc/dxcapi.h"
#include "../External/Imgui/imgui.h"

#include "DescriptorHeap.h"

#include "AccelerationStructureManager.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include "Texture.h"

#include "ShaderBindingTable.h"
#include "RaytracingPipelineState.h"
#include "RootSignature.h"

#include "Model.h"

using namespace DirectX;

using float2 = DirectX::XMFLOAT2;
using float3 = DirectX::XMFLOAT3;
using float4 = DirectX::XMFLOAT4;

struct Transform
{
	DirectX::XMFLOAT3 position_{ 0.0f, 0.0f, 500.0f };
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

	void OnGui()
	{
		if (ImGui::TreeNode("Transform"))
		{
			ImGui::DragFloat3("Position", &position_.x, 0.01f, -FLT_MAX, FLT_MAX);
			ImGui::DragFloat3("Scaling", &scaling_.x, 0.01f, -FLT_MAX, FLT_MAX);
			ImGui::DragFloat3("Rotation", &rotation_.x, 3.14f / 180.0f * 0.1f, -FLT_MAX, FLT_MAX);
			ImGui::SliderInt("CoordinateSystem", &coordinate_system_index_, 0, 3);
			ImGui::Text(kCoordinateSystemStr[coordinate_system_index_].c_str());
			ImGui::TreePop();
		}
	}
};

struct Vertex
{
	float3 position_;
	float3 normal_;
	float3 tangent_;
	float3 binormal_;
	float2 texcoord_;
};

namespace argent::graphics
{
	class GraphicsDevice;
	class GraphicsCommandList;
	class CommandQueue;

	class Raytracer
	{
	public:
		static constexpr UINT kNoModelGeometryCounts = 2u;
		enum GeometryType
		{
			Plane,
			Sphere,

			CoralRock,
			Coral0,
			Coral4,
			Coral5,
			Coral6,
			Coral7,
			Coral8,

			GoldDome,

			Basket0,
			WoodTable0,
			TreasureChest,
			Skull,

			Flower0,
			Flower1,
			Flower2,
			Flower3,
			GeometryTypeCount,
		};

	private:
		std::string name[GeometryTypeCount]
		{
			"Plane",
			"Sphere",

			"CoralRock",
			"Coral0",
			"Coral4",
			"Coral5",
			"Coral6",
			"Coral7",
			"Coral8",

			"GoldDome",

			"Basket0",
			"WoodTable",
			"TreasureChest",
			"Skull",

			"Flower0",
			"Flower1",
			"Flower2",
			"Flower3",


		};

		std::vector<std::wstring> kHitGroupName/*[GeometryTypeCount]*/
		{
			L"HG_Plane",
			L"HG_Sphere",

			L"HG_CoralRock",
			L"HG_Coral0",
			L"HG_Coral4",
			L"HG_Coral5",
			L"HG_Coral6",
			L"HG_Coral7",
			L"HG_Coral8",

			L"HG_GoldDome",

			L"HG_Basket0",
			L"HG_WoodTable",
			L"HG_TreasureChest",
			L"HG_Skull",

			L"HG_Flower0",
			L"HG_Flower1",
			L"HG_Flower2",
			L"HG_Flower3",
		};

		std::string filepaths[GeometryTypeCount - kNoModelGeometryCounts]
		{
			"./Assets/Model/CoralRock.fbx",
			"./Assets/Model/Coral_0.fbx",
			"./Assets/Model/Coral_4.fbx",
			"./Assets/Model/Coral_5.fbx",
			"./Assets/Model/Coral_6.fbx",
			"./Assets/Model/Coral_7.fbx",
			"./Assets/Model/Coral_8.fbx",

			"./Assets/Model/GoldDome.fbx",

			"./Assets/Model/WeaverBasket_0.fbx",
			"./Assets/Model/TableWood_0.fbx",
			"./Assets/Model/TreasureChest.fbx",
			"./Assets/Model/SkeletonSkull.fbx",

			"./Assets/Model/Flower_0.fbx",
			"./Assets/Model/Flower_1.fbx",
			"./Assets/Model/Flower_2.fbx",
			"./Assets/Model/Flower_3.fbx",
		};


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
		void Shutdown();

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

		void FbxLoader(const char* filename);
	private:
		//Shader
		Microsoft::WRL::ComPtr<IDxcBlob> ray_gen_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> miss_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> plane_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> static_mesh_library_;

		Microsoft::WRL::ComPtr<IDxcBlob> sphere_intersection_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> sphere_library_;


		//Output buffer
		Microsoft::WRL::ComPtr<ID3D12Resource> output_buffer_;
		Descriptor output_descriptor_;
		Descriptor tlas_result_descriptor_;
		
		UINT64 width_;
		UINT height_;

		std::unique_ptr<VertexBuffer> vertex_buffers_[kNoModelGeometryCounts];
		std::unique_ptr<IndexBuffer> index_buffers_[kNoModelGeometryCounts];

		//Descriptor cube_vertex_descriptor_;
		//Descriptor cube_index_descriptor_;


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
			DirectX::XMFLOAT4 texcoord_offset_;

			void OnGui()
			{
				if (ImGui::TreeNode("Material"))
				{
					ImGuiColorEditFlags flags =
						ImGuiColorEditFlags_PickerHueWheel |
						ImGuiColorEditFlags_NoInputs |
						ImGuiColorEditFlags_NoAlpha |
						ImGuiColorEditFlags_NoOptions |
						ImGuiColorEditFlags_NoTooltip |
						ImGuiColorEditFlags_NoSidePreview |
						ImGuiColorEditFlags_NoDragDrop |
						ImGuiColorEditFlags_NoBorder
						;
					ImGui::ColorEdit3("Color Edit", &albedo_color_.x, flags);
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
			ObjectCbv,		//CBV
			MaterialCbv,	//CBV
			AlbedoTexture,		//SRV
			NormalTexture,		//SRV
			VertexBufferGpuDescriptorHandle,	//SRV
			RootSignatureBinderCount,
		};

		Transform transforms_[GeometryType::GeometryTypeCount];
		uint8_t* world_mat_map_;
		Microsoft::WRL::ComPtr<ID3D12Resource> world_matrix_buffer_;

		Descriptor object_descriptor_;

		uint8_t* material_map_;
		Microsoft::WRL::ComPtr<ID3D12Resource> material_buffer_;
		Material materials_[kNoModelGeometryCounts];

		UINT hit_shader_table_size_;
		UINT hit_shader_table_stride_;

		dxr::AccelerationStructureManager as_manager_;
		UINT tlas_unique_id_[GeometryTypeCount];


		static constexpr int kSkymapCounts = 4;
		std::unique_ptr<Texture> skymaps_[kSkymapCounts];
		int skymap_index_ = 0;
		Microsoft::WRL::ComPtr<ID3D12Resource> skymap_index_buffer_;
		int* map_skymap_index_;

		bool is_wait_ = false;
	public:

		struct Mesh
		{
			uint64_t unique_id_{};
			std::string name_;
			int64_t node_index_{};

			std::vector<Vertex> vertices_;
			std::vector<uint32_t> indices_;

			std::unique_ptr<VertexBuffer> vertex_buffer_;
			std::unique_ptr<IndexBuffer> index_buffer_;
		};

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> blas_transform_cube_;
		std::vector<Mesh> meshes_;


		//For Shader Binding Table
		dxr::ShaderBindingTable raygen_shader_table_;
		dxr::ShaderBindingTable miss_shader_table_;
		dxr::ShaderBindingTable hit_group_shader_table_;

		dxr::RaytracingPipelineState pipeline_state_;

		std::shared_ptr<game_resource::Model> model_[GeometryTypeCount - kNoModelGeometryCounts];

		RootSignature global_root_signature_;
		RootSignature raygen_miss_root_signature_;
		RootSignature hit_group_root_signature_;

	};
}