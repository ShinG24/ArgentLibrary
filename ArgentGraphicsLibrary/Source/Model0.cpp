#include "../Inc/Model0.h"

#include <filesystem>
#include <fstream>

#include "../Inc/GraphicsDevice.h"
#include "../External/Imgui/imgui.h"





namespace argent::game_resource
{
	Mesh::Mesh(std::string name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices):
		name_(name)
	,	vertices_(vertices)
	,	indices_(indices)
	{}

	void Mesh::Awake(const graphics::dx12::GraphicsDevice* graphics_device, graphics::dx12::DescriptorHeap* srv_descriptor_heap)
	{
		vertex_buffer_ = std::make_unique<graphics::dx12::VertexBuffer>(graphics_device, vertices_.data(), sizeof(Vertex), vertices_.size());
		index_buffer_ = std::make_unique<graphics::dx12::IndexBuffer>(graphics_device, indices_.data(), static_cast<UINT>(indices_.size()));

		vertex_srv_descriptor_ = srv_descriptor_heap->PopDescriptor();
		index_srv_descriptor_ = srv_descriptor_heap->PopDescriptor();
		graphics_device->CreateBufferSRV(vertex_buffer_->GetBufferObject(), vertex_buffer_->GetVertexCounts(), 
			sizeof(Vertex), vertex_srv_descriptor_.cpu_handle_);
		graphics_device->CreateBufferSRV(index_buffer_->GetBufferObject(), index_buffer_->GetIndexCounts(), 
			sizeof(uint32_t), index_srv_descriptor_.cpu_handle_);
	}

	void Mesh::OnGui()
	{
		if(ImGui::TreeNode("Mesh"))
		{
			ImGui::Text(name_.c_str());
			int vertex_counts = vertex_buffer_->GetVertexCounts();
			int index_counts = index_buffer_->GetIndexCounts();
			ImGui::InputInt("Vertex Counts", &vertex_counts);
			ImGui::InputInt("Index Counts", &index_counts);
			ImGui::TreePop();
		}
	}

	Material::Material(std::string name, std::string albedo_texture_name, std::string normal_texture_name) :
		name_(name)
	,	albedo_texture_name_(albedo_texture_name)
	,	normal_texture_name_(normal_texture_name)
	{}

	void Material::Awake(const graphics::dx12::GraphicsDevice* graphics_device, const graphics::dx12::CommandQueue* command_queue, graphics::dx12::DescriptorHeap* srv_heap)
	{
		albedo_texture_path_replacement_.resize(256);
		normal_texture_path_replacement_.resize(256);

		std::filesystem::path albedo = albedo_texture_name_;
		std::filesystem::path normal = normal_texture_name_;
		albedo_texture_ = std::make_unique<graphics::Texture>(graphics_device, command_queue, srv_heap, albedo.wstring().c_str());
		normal_texture_ = std::make_unique<graphics::Texture>(graphics_device, command_queue, srv_heap, normal.wstring().c_str());
		constant_buffer_ = std::make_unique<graphics::dx12::ConstantBuffer>(graphics_device, sizeof(Constant), 1u);
		constant_buffer_->CopyToGpu(&data_, 0u);
	}

	void Material::WaitBeforeUse()
	{
		if(albedo_texture_) albedo_texture_->WaitBeforeUse();
		if(normal_texture_) normal_texture_->WaitBeforeUse();
	}

	void Material::OnGui()
	{
		if(ImGui::TreeNode("Material"))
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
			ImGui::ColorEdit3("Color Edit", &data_.color_.x, flags);
			ImGui::DragFloat("Diffuse Coef", &data_.diffuse_coefficient_, 0.001f, 0.0f, 1.0f);
			ImGui::DragFloat("Specular Coef", &data_.specular_coefficient_, 0.001f, 0.0f, 1.0f);
			ImGui::DragFloat("Reflectance Coef", &data_.reflectance_coefficient_, 0.001f, 0.0f, 1.0f);
			ImGui::DragFloat("Specular Power", &data_.specular_power_, 0.1f, 0.0f, 100.0f);
			ImGui::DragFloat2("Texcoord Offset", &data_.texcoord_offset_.x, 0.001f, 0.0, 1.0f);

			if(ImGui::TreeNode("Texture"))
			{
				if(!albedo_texture_name_.empty())
				{
					ImGui::Text("Albedo");
					ImGui::Image(reinterpret_cast<ImTextureID>(GetAlbedoTextureGpuHandle().ptr), ImVec2(256, 256));
				}
				if(!normal_texture_name_.empty())
				{
					ImGui::Text("Normal");	
					ImGui::Image(reinterpret_cast<ImTextureID>(GetNormalTextureGpuHandle().ptr), ImVec2(256, 256));
				}
		#ifdef _DEBUG
				ImGui::Text(albedo_texture_name_.c_str());
				ImGui::InputText("AlbedoTextureFilePath", &albedo_texture_path_replacement_.at(0), albedo_texture_path_replacement_.capacity());
				if(ImGui::Button("Accept Albedo"))
				{
					albedo_texture_name_ = albedo_texture_path_replacement_;
				}

				ImGui::Text(normal_texture_name_.c_str());
				ImGui::InputText("NormalTextureFilePath", &normal_texture_path_replacement_.at(0), normal_texture_path_replacement_.capacity());
				if(ImGui::Button("Accept Normal"))
				{
					normal_texture_name_ = normal_texture_path_replacement_;
				}


	#endif
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}

	}

	Model::Model(std::string filepath,std::string mesh_name, const std::vector<Mesh::Vertex>& vertices, const std::vector<uint32_t>& indices,
		const std::string& albedo_texture_name, const std::string& normal_texture_name):
		filepath_(filepath)
	{
		mesh_ = std::make_shared<Mesh>(mesh_name, vertices, indices);
		material_ = std::make_shared<Material>("Material", albedo_texture_name, normal_texture_name);
	}

	Model::Model(std::vector<std::shared_ptr<Mesh>> mesh_vec, std::vector<std::shared_ptr<Material>> material_vec):
		mesh_vec_(mesh_vec)
	,	material_vec_(material_vec)
	{}

	void Model::Awake(const graphics::dx12::GraphicsDevice* graphics_device, const graphics::dx12::CommandQueue* command_queue, graphics::dx12::DescriptorHeap* srv_heap)
	{
		//TODO
		mesh_->Awake(graphics_device, srv_heap);
		material_->Awake(graphics_device, command_queue, srv_heap);

		shader_binding_data_.resize(Counts);
		shader_binding_data_.at(MaterialCbv) = reinterpret_cast<void*>(material_->GetMaterialConstantBufferLocation());
		shader_binding_data_.at(AlbedoTexture) = reinterpret_cast<void*>(material_->GetAlbedoTextureGpuHandle().ptr);
		shader_binding_data_.at(NormalTexture) = reinterpret_cast<void*>(material_->GetNormalTextureGpuHandle().ptr);
		shader_binding_data_.at(VertexBufferGpuDescriptorHandle) = reinterpret_cast<void*>(mesh_->GetVertexGpuHandle().ptr);

		for(auto& m : mesh_vec_)
		{
			m->Awake(graphics_device, srv_heap);
		}

		for(auto& m : material_vec_)
		{
			m->Awake(graphics_device, command_queue, srv_heap);
		}
	}

	void Model::WaitBeforeUse()
	{
		material_->WaitBeforeUse();
	}

	void Model::UpdateMaterialData()
	{
		material_->CopyToGpu();
	}

	void Model::OnGui()
	{
		std::filesystem::path path = filepath_;

		if(ImGui::TreeNode(path.stem().string().c_str()))
		{
			mesh_->OnGui();
			material_->OnGui();
			
			ImGui::TreePop();
		}
	}
}
