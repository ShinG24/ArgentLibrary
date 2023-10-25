#pragma once

#include <DirectXMath.h>

#include <memory>
#include <string>
#include <vector>

#include <cereal/archives/binary.hpp>
#include<cereal/types/memory.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include "Texture.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "DescriptorHeap.h"

namespace DirectX
{
	template<class T>
	void serialize(T& archive, XMFLOAT2& v)
	{
		archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y));
	}
	template<class T>
	void serialize(T& archive, XMFLOAT3& v)
	{
		archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y), cereal::make_nvp("z", v.z));
	}
	template<class T>
	void serialize(T& archive, XMFLOAT4& v)
	{
		archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y), cereal::make_nvp("z", v.z), cereal::make_nvp("w", v.w));
	}
	template<class T>
	void serialize(T& archive, XMFLOAT4X4& m)
	{
		archive(
		 cereal::make_nvp("_11", m._11), cereal::make_nvp("_12", m._12),
		 cereal::make_nvp("_13", m._13), cereal::make_nvp("_14", m._14),
		 cereal::make_nvp("_21", m._21), cereal::make_nvp("_22", m._22),
		 cereal::make_nvp("_23", m._23), cereal::make_nvp("_24", m._24),
		 cereal::make_nvp("_31", m._31), cereal::make_nvp("_32", m._32),
		 cereal::make_nvp("_33", m._33), cereal::make_nvp("_34", m._34),
		 cereal::make_nvp("_41", m._41), cereal::make_nvp("_42", m._42),
		 cereal::make_nvp("_43", m._43), cereal::make_nvp("_44", m._44)
		 );
	}
}


namespace argent::graphics
{
	class GraphicsDevice;
}

namespace argent::game_resource
{
	class Mesh
	{
	public:
		struct Vertex
		{
			DirectX::XMFLOAT3 position_;
			DirectX::XMFLOAT3 normal_;
			DirectX::XMFLOAT3 tangent_;
			DirectX::XMFLOAT3 binormal_;
			DirectX::XMFLOAT2 texcoord_;
			template<class T>
			void serialize(T& archive)
			{
				archive(position_, normal_, tangent_, binormal_, texcoord_);
			}
		};

		template <class Archive>
		void serialize(Archive& archive)
		{
			archive(name_, vertices_, indices_);
		}

	public:
		Mesh() = default;
		Mesh(std::string name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		~Mesh() = default;

		Mesh(const Mesh&) = delete;
		Mesh(const Mesh&&) = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(const Mesh&&) = delete;

		void Awake(const graphics::GraphicsDevice* graphics_device,
			graphics::DescriptorHeap* srv_descriptor_heap);

		void OnGui();

		graphics::VertexBuffer* GetVertexBuffer() const { return vertex_buffer_.get(); }
		graphics::IndexBuffer* GetIndexBuffer() const { return index_buffer_.get(); }

		D3D12_GPU_VIRTUAL_ADDRESS GetVertexBufferLocation() const
			{ return vertex_buffer_->GetBufferLocation(); }
		D3D12_GPU_VIRTUAL_ADDRESS GetIndexBufferLocation() const { return index_buffer_->GetBufferObject()->GetGPUVirtualAddress(); }

		D3D12_GPU_DESCRIPTOR_HANDLE GetVertexGpuHandle() const { return vertex_srv_descriptor_.gpu_handle_; }
		D3D12_GPU_DESCRIPTOR_HANDLE GetIndexGpuHandle() const { return index_srv_descriptor_.gpu_handle_; }

	private:
		std::string name_;
		std::unique_ptr<graphics::VertexBuffer> vertex_buffer_;
		std::unique_ptr<graphics::IndexBuffer> index_buffer_;
		std::vector<Vertex> vertices_;
		std::vector<uint32_t> indices_;
		graphics::Descriptor vertex_srv_descriptor_{};
		graphics::Descriptor index_srv_descriptor_{};
	};

	class Material
	{
	public:

		struct Constant
		{
			template <class Archive>
			void serialize(Archive& archive)
			{
				archive(color_, diffuse_coefficient_, specular_coefficient_, reflectance_coefficient_, specular_power_);
			}

			DirectX::XMFLOAT4 color_{1, 1, 1, 1};
			float diffuse_coefficient_{ 0.8f };
			float specular_coefficient_{ 0.3f };
			float reflectance_coefficient_{ 0.3f};
			float specular_power_{ 50.0f };
		};

		template <class Archive>
		void serialize(Archive& archive)
		{
			archive(albedo_texture_name_, normal_texture_name_, data_);
		}

	public:
		Material() = default;
		Material(std::string name, std::string albedo_texture_name, std::string normal_texture_name);

		void Awake(const graphics::GraphicsDevice* graphics_device, const graphics::CommandQueue* command_queue, graphics::DescriptorHeap* srv_heap);

		void OnGui();

		void CopyToGpu()
		{
			constant_buffer_->CopyToGpu(data_, 0);
		}
		D3D12_GPU_DESCRIPTOR_HANDLE GetAlbedoTextureGpuHandle() const
		{
			return albedo_texture_ ? albedo_texture_->GetGpuHandle() : static_cast<D3D12_GPU_DESCRIPTOR_HANDLE>(0);
		}
		D3D12_GPU_DESCRIPTOR_HANDLE GetNormalTextureGpuHandle() const
		{
			return normal_texture_ ? normal_texture_->GetGpuHandle() : static_cast<D3D12_GPU_DESCRIPTOR_HANDLE>(0);
		}
		D3D12_GPU_VIRTUAL_ADDRESS GetMaterialConstantBufferLocation() const
		{
			return constant_buffer_->GetGpuVirtualAddress(0);
		}
	private:
		std::string name_;
		std::string albedo_texture_name_;
		std::string normal_texture_name_;
		std::string albedo_texture_path_replacement_;
		std::string normal_texture_path_replacement_;

		std::unique_ptr<graphics::Texture> albedo_texture_{};
		std::unique_ptr<graphics::Texture> normal_texture_{};
		std::unique_ptr<graphics::ConstantBuffer<Constant>> constant_buffer_{};
		Constant data_;
	};

	class Model
	{
	public:
		enum RootSignatureBinding
		{
			MaterialCbv,
			AlbedoTexture,
			NormalTexture,
			VertexBufferGpuDescriptorHandle,
			Counts,
		};

		template <class Archive>
		void serialize(Archive& archive)
		{
			archive(filepath_, mesh_, material_);
		}

	public:
		Model() = default;

		Model(std::string filepath,std::string mesh_name, const std::vector<Mesh::Vertex>& vertices, const std::vector<uint32_t>& indices,
		const std::string& albedo_texture_name, const std::string& normal_texture_name);

		void Awake(const graphics::GraphicsDevice* graphics_device, const graphics::CommandQueue* command_queue, graphics::DescriptorHeap* srv_heap);
		const std::vector<void*>& GetShaderBindingData() { return shader_binding_data_; }

		void UpdateMaterialData();

		std::string GetFilePath() const { return filepath_; }
		void OnGui();

		std::shared_ptr<Mesh> GetMesh() const { return mesh_; }
		std::shared_ptr<Material> GetMaterial() const { return material_; }
	private:
		std::string filepath_;
		std::vector<void*> shader_binding_data_;
		std::shared_ptr<Mesh> mesh_;
		std::shared_ptr<Material> material_;
	};
}