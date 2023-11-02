#include "Subsystem/Graphics/Resource/Mesh.h"

#include <imgui.h>

#include "Subsystem/Graphics/Wrapper/DXR/AccelerationStructureManager.h"
#include "Subsystem/Graphics/API/D3D12/GraphicsDevice.h"
#include "Subsystem/Graphics/API/D3D12/DescriptorHeap.h"
#include "Subsystem/Graphics/API/D3D12/CommandQueue.h"
#include "Subsystem/Graphics/API/D3D12/VertexBuffer.h"
#include "Subsystem/Graphics/API/D3D12/IndexBuffer.h"
	
#include "Subsystem/Graphics/Common/GraphicsContext.h"
		  
#include "Subsystem/Graphics/Resource/Material.h"


//#include "../Inc/BottomLevelAccelerationStructure.h"

namespace argent::graphics
{
	Mesh::Data::Data(std::vector<Position>& position_vec, std::vector<Normal> normal_vec,
		std::vector<Tangent>& tangent_vec, std::vector<Binormal>& binormal_vec, std::vector<Texcoord>& texcoord_vec,
		std::vector<uint32_t>& index_vec, const DirectX::XMFLOAT4X4& default_global_transform,
		std::shared_ptr<Material> material) :
		position_vec_(std::move(position_vec))
	,	normal_vec_(std::move(normal_vec))
	,	tangent_vec_(std::move(tangent_vec))
	,	binormal_vec_(std::move(binormal_vec))
	,	texcoord_vec_(std::move(texcoord_vec))
	,	index_vec_(std::move(index_vec))
	,	default_global_transform_(default_global_transform)
	,	material_(material)
	{}


	bool Mesh::Data::HasNullData() const
	{
		return position_vec_.empty() || normal_vec_.empty() || tangent_vec_.empty() ||
			binormal_vec_.empty() || texcoord_vec_.empty() || index_vec_.empty();
	}

	void Mesh::Awake(const GraphicsContext* graphics_context)
	{
		//すべてのデータを使う前提のため一つでも.empty()なデータが有る場合はアサートを出す
		if (data_.HasNullData()) _ASSERT_EXPR(FALSE, L"The Mesh has one or more null data");

		//頂点バッファ、インデックスバッファ、サポートしてる場合はBLAS
		CreateComObject(graphics_context);
	}

	void Mesh::OnGui()
	{
		if(GetName().empty()) _ASSERT_EXPR(FALSE, L"The Mesh name is null");
		if(ImGui::TreeNode(GetName().c_str()))
		{
			int num_vertex = static_cast<int>(data_.position_vec_.size());
			int num_index = static_cast<int>(data_.index_vec_.size());
			ImGui::InputInt("Rendering Vertex", &num_vertex, 0, 0, ImGuiInputTextFlags_ReadOnly);
			ImGui::InputInt("Rendering Index", &num_index, 0, 0, ImGuiInputTextFlags_ReadOnly);
			ImGui::TreePop();
		}
	}

	void Mesh::CreateComObject(const GraphicsContext* graphics_context)
	{
		//Vertex & Index Buffer作成
		position_buffer_ = std::make_unique<dx12::VertexBuffer>(graphics_context->graphics_device_,
			data_.position_vec_.data(), sizeof(Position), data_.position_vec_.size());
		normal_buffer_ = std::make_unique<dx12::VertexBuffer>(graphics_context->graphics_device_, data_.normal_vec_.data(),
			sizeof(Normal), data_.normal_vec_.size());
		tangent_buffer_ = std::make_unique<dx12::VertexBuffer>(graphics_context->graphics_device_, 
			data_.tangent_vec_.data(), sizeof(Tangent), data_.tangent_vec_.size());
		binormal_buffer_ = std::make_unique<dx12::VertexBuffer>(graphics_context->graphics_device_, data_.binormal_vec_.data(),
			sizeof(Binormal), data_.binormal_vec_.size());
		texcoord_buffer_ = std::make_unique<dx12::VertexBuffer>(graphics_context->graphics_device_, 
			data_.texcoord_vec_.data(), sizeof(Texcoord), data_.texcoord_vec_.size());

		index_buffer_ = std::make_unique<dx12::IndexBuffer>(graphics_context->graphics_device_, 
			data_.index_vec_.data(), static_cast<UINT>(data_.index_vec_.size()));

		//For Raytracing
		CreateRaytracingObject(graphics_context);
	}

	void Mesh::CreateRaytracingObject(const GraphicsContext* graphics_context)
	{
		//レイトレサポート外の場合は何もしない
		if(!graphics_context->graphics_device_->IsDxrSupported()) return;

		//レイトレーシング用　Shaderにバインドするために頂点バッファのSRVを作る
		position_srv_descriptor_ = graphics_context->cbv_srv_uav_descriptor_heap_->PopDescriptor();
		normal_srv_descriptor_ = graphics_context->cbv_srv_uav_descriptor_heap_->PopDescriptor();
		tangent_srv_descriptor_ = graphics_context->cbv_srv_uav_descriptor_heap_->PopDescriptor();
		binormal_srv_descriptor_ = graphics_context->cbv_srv_uav_descriptor_heap_->PopDescriptor();
		texcoord_srv_descriptor_ = graphics_context->cbv_srv_uav_descriptor_heap_->PopDescriptor();
		index_srv_descriptor_ = graphics_context->cbv_srv_uav_descriptor_heap_->PopDescriptor();

		graphics_context->graphics_device_->CreateBufferSRV(position_buffer_->GetBufferObject(),
			position_buffer_->GetVertexCounts(), sizeof(Position), position_srv_descriptor_.cpu_handle_);
		graphics_context->graphics_device_->CreateBufferSRV(normal_buffer_->GetBufferObject(),
			normal_buffer_->GetVertexCounts(), sizeof(Normal), normal_srv_descriptor_.cpu_handle_);
		graphics_context->graphics_device_->CreateBufferSRV(tangent_buffer_->GetBufferObject(),
			tangent_buffer_->GetVertexCounts(), sizeof(Tangent), tangent_srv_descriptor_.cpu_handle_);
		graphics_context->graphics_device_->CreateBufferSRV(binormal_buffer_->GetBufferObject(),
			binormal_buffer_->GetVertexCounts(), sizeof(Binormal), binormal_srv_descriptor_.cpu_handle_);
		graphics_context->graphics_device_->CreateBufferSRV(texcoord_buffer_->GetBufferObject(),
			texcoord_buffer_->GetVertexCounts(), sizeof(Texcoord), texcoord_srv_descriptor_.cpu_handle_);
		graphics_context->graphics_device_->CreateBufferSRV(index_buffer_->GetBufferObject(),
			index_buffer_->GetIndexCounts(), sizeof(uint32_t), index_srv_descriptor_.cpu_handle_);

		//Bottom Level Acceleration Structure
		dx12::BLASBuildDesc build_desc{};
		build_desc.vertex_buffer_vec_.emplace_back(position_buffer_.get());
		build_desc.index_buffer_vec_.emplace_back(index_buffer_.get());
		//TODO Transformについて

		blas_unique_id_ = graphics_context->as_manager_->AddBottomLevelAS(graphics_context->graphics_device_, 
			graphics_context->resource_upload_command_list_, &build_desc, true);

	}
}
