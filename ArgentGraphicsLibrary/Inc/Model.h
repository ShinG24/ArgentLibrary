#pragma once

#include <DirectXMath.h>

#include <memory>
#include <vector>

#include "Texture.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "DescriptorHeap.h"

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
		};
	public:

		Mesh() = default;
		~Mesh() = default;

		Mesh(const Mesh&) = delete;
		Mesh(const Mesh&&) = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(const Mesh&&) = delete;

		void Awake(const graphics::GraphicsDevice* graphics_device,
			graphics::DescriptorHeap* srv_descriptor_heap);

		D3D12_GPU_VIRTUAL_ADDRESS GetVertexBufferLocation() const
			{ return vertex_buffer_->GetBufferLocation(); }
		D3D12_GPU_VIRTUAL_ADDRESS GetIndexBufferLocation() const { return index_buffer_->GetBufferObject()->GetGPUVirtualAddress(); }

		D3D12_GPU_DESCRIPTOR_HANDLE GetVertexGpuHandle() const { return vertex_srv_descriptor_.gpu_handle_; }
		D3D12_GPU_DESCRIPTOR_HANDLE GetIndexGpuHandle() const { return index_srv_descriptor_.gpu_handle_; }

	private:
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
			DirectX::XMFLOAT4 color_{1, 1, 1, 1};
			float diffuse_coefficient_{ 0.8f };
			float specular_coefficient_{ 0.3f };
			float reflectance_coefficient_{ 0.3f};
			float specular_power_{ 50.0f };
		};

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
		
		std::unique_ptr<graphics::Texture> albedo_texture_{};
		std::unique_ptr<graphics::Texture> normal_texture_{};
		std::unique_ptr<graphics::ConstantBuffer<Constant>> constant_buffer_{};
		Constant data_;
	};

	class Model
	{
	public:

	private:
		Mesh mesh_;
		Material material_;
	};
}