#include "../Inc/Model.h"

#include <filesystem>

#include "../External/Imgui/imgui.h"

#include "../Inc/GraphicsDevice.h"
#include "../Inc/GraphicsContext.h"

#include "../Inc/VertexBuffer.h"
#include "../Inc/IndexBuffer.h"

#include "../Inc/Mesh.h"
#include "../Inc/Material.h"
#include "../Inc/StandardMaterial.h"

namespace argent::graphics
{
	//ファイルネームの抽出
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
	{
		if(mesh_vec_.empty() || material_vec_.empty())  _ASSERT_EXPR(FALSE, L"Mesh and Material can not be null");
	}

	//初期化関数
	void Model::Awake(const GraphicsContext* graphics_context) const
	{
		if(mesh_vec_.empty() || material_vec_.empty())  _ASSERT_EXPR(FALSE, L"Mesh and Material can not be null");

		for (auto& m : mesh_vec_)
		{
			m->Awake(graphics_context);
		}

		for(auto& m : material_vec_)
		{
			m->Awake(graphics_context);
		}
	}

	//Guiへの描画
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
				//Const参照でも大丈夫なのか
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
