#include "../Inc/StandardMaterial.h"

#include "../External/Imgui/imgui.h"

#include "../Inc/Texture.h"


namespace argent::graphics
{
	StandardMaterial::StandardMaterial(const std::string& name, const Data& data) :
		Material(name)
	,	data_(data)
	,	constant_data_(data.metallic_factor_, data.smoothness_factor_)
	{}

	void StandardMaterial::Awake(const GraphicsContext* graphics_context)
	{
		for(const auto& map : data_.filepath_map_)
		{
			texture_map_[map.first] = std::make_unique<Texture>(graphics_context, map.second.c_str());
		}
	}

	void StandardMaterial::OnGui()
	{
		if(ImGui::TreeNode(GetName().c_str()))
		{
			//TODO スタンダードマテリアルのGui処理
			ImGui::TreePop();
		}
	}
}
