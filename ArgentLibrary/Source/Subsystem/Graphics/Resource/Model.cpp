#include "Subsystem/Graphics/Resource/Model.h"

#include <filesystem>

#include <imgui.h>

#include "Subsystem/Graphics/API/D3D12/GraphicsDevice.h"
#include "Subsystem/Graphics/API/D3D12/DescriptorHeap.h"
#include "Subsystem/Graphics/API/D3D12/VertexBuffer.h"
#include "Subsystem/Graphics/API/D3D12/IndexBuffer.h"

#include "Subsystem/Graphics/Common/GraphicsContext.h"

#include "Subsystem/Graphics/Resource/Mesh.h"
#include "Subsystem/Graphics/Resource/Material.h"
#include "Subsystem/Graphics/Resource/StandardMaterial.h"


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
	void Model::Awake(const GraphicsContext* graphics_context)
	{
		if(is_awake_) return;

		if(mesh_vec_.empty() || material_vec_.empty())  _ASSERT_EXPR(FALSE, L"Mesh and Material can not be null");

		for (auto& m : mesh_vec_)
		{
			m->Awake(graphics_context);
		}

		for(auto& m : material_vec_)
		{
			m->Awake(graphics_context);
		}

		is_awake_ = true;
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
