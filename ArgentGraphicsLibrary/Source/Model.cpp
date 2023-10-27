#include "../Inc/Model.h"

#include <filesystem>

#include "../External/Imgui/imgui.h"

#include "../Inc/GraphicsDevice.h"
#include "../Inc/GraphicsContext.h"

#include "../Inc/VertexBuffer.h"
#include "../Inc/IndexBuffer.h"

#include "../Inc/Mesh.h"
#include "../Inc/Material.h"

namespace argent::graphics
{
	//�t�@�C���l�[���̒��o
	inline const char* Stem(const std::string& value)
	{
		std::filesystem::path path = value;
		return path.stem().string().c_str();
	}

	Model::Model(std::string filepath, std::vector<std::shared_ptr<Mesh>>& mesh_vec,
		std::vector<std::shared_ptr<Material>>& material_vec) :
		GameResource(Stem(filepath))
	,	filepath_(std::move(filepath))
	,	mesh_vec_(std::move(mesh_vec))
	,	material_vec_(std::move(material_vec))
	{}

	//�������֐�
	void Model::Awake(const GraphicsContext* graphics_context) const
	{
		for (auto& m : mesh_vec_)
		{
			m->Awake(graphics_context);
		}

		for(auto& m : material_vec_)
		{
			m->Awake(graphics_context);
		}
	}

	//Gui�ւ̕`��
	void Model::OnGui()
	{
		if(ImGui::TreeNode("Model"))
		{
			ImGui::Text(filepath_.c_str());
			if(ImGui::TreeNode("Mesh"))
			{
				for(const auto& m : mesh_vec_)
				{
					m->OnGui();
				}
				ImGui::TreePop();
			}
			if(ImGui::TreeNode("Material"))
			{
				//Const�Q�Ƃł����v�Ȃ̂�
				for(const auto& m : material_vec_)
				{
					m->OnGui();
				}
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
	}
}
