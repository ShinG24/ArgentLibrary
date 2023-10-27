#include "../Inc/Mesh.h"

#include "../External/Imgui/imgui.h"

#include "../Inc/GraphicsContext.h"

#include "../Inc/GraphicsDevice.h"
#include "../Inc/VertexBuffer.h"
#include "../Inc/IndexBuffer.h"
#include "../Inc/Material.h"


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

		//TODO BLASのクラスを作り直す必要あり
		//Raytracingをサポートしている場合のみ
		//blas_ = std::make_unique<dx12::BottomLevelAccelerationStructure>()
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
}
