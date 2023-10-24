#include "../Inc/Model.h"


namespace argent::game_resource
{
	void Mesh::Awake(const graphics::GraphicsDevice* graphics_device, graphics::DescriptorHeap* srv_descriptor_heap)
	{
		vertex_buffer_ = std::make_unique<graphics::VertexBuffer>(graphics_device, vertices_.data(), sizeof(Vertex), vertices_.size());
		index_buffer_ = std::make_unique<graphics::IndexBuffer>(graphics_device, indices_.data(), indices_.size());

		vertex_srv_descriptor_ = srv_descriptor_heap->PopDescriptor();
		index_srv_descriptor_ = srv_descriptor_heap->PopDescriptor();
		graphics_device->CreateBufferSRV(vertex_buffer_->GetBufferObject(), vertex_buffer_->GetVertexCounts(), 
			sizeof(Vertex), vertex_srv_descriptor_.cpu_handle_);
		graphics_device->CreateBufferSRV(index_buffer_->GetBufferObject(), index_buffer_->GetIndexCounts(), 
			sizeof(uint32_t), index_srv_descriptor_.cpu_handle_);
	}
}
