#include "../Inc/Mesh.h"

#include "../Inc/GraphicsContext.h"

#include "../Inc/GraphicsDevice.h"
#include "../Inc/VertexBuffer.h"
#include "../Inc/IndexBuffer.h"
#include "../Inc/BottomLevelAccelerationStructure.h"

namespace argent::graphics
{
	bool Mesh::Data::HasNullData() const
	{
		return position_vec_.empty() || normal_vec_.empty() || tangent_vec_.empty() ||
			binormal_vec_.empty() || texcoord_vec_.empty() || index_vec_.empty();
	}

	void Mesh::Awake(const GraphicsContext* graphics_context)
	{
		if (data_.HasNullData()) _ASSERT_EXPR(FALSE, L"The Mesh has one or more null data");

		//Vertex & Index Buffer作成
		position_buffer_ = std::make_unique<dx12::VertexBuffer>(graphics_context->graphics_device_,
			data_.position_vec_.data(), sizeof(Position), data_.position_vec_.size());
		normal_buffer_ = std::make_unique<dx12::VertexBuffer>(graphics_context->graphics_device_, data_.normal_vec_.data(),
			sizeof(Position), data_.normal_vec_.size());
		tangent_buffer_ = std::make_unique<dx12::VertexBuffer>(graphics_context->graphics_device_, 
			data_.tangent_vec_.data(), sizeof(Position), data_.tangent_vec_.size());
		binormal_buffer_ = std::make_unique<dx12::VertexBuffer>(graphics_context->graphics_device_, data_.binormal_vec_.data(),
			sizeof(Position), data_.binormal_vec_.size());
		texcoord_buffer_ = std::make_unique<dx12::VertexBuffer>(graphics_context->graphics_device_, 
			data_.texcoord_vec_.data(), sizeof(Position), data_.texcoord_vec_.size());

		index_buffer_ = std::make_unique<dx12::IndexBuffer>(graphics_context->graphics_device_, 
			data_.index_vec_.data(), data_.index_vec_.size());

		//TODO BLASのクラスを作り直す必要あり
		//Raytracingをサポートしている場合のみ
		//blas_ = std::make_unique<dx12::BottomLevelAccelerationStructure>()
	}
		
}
