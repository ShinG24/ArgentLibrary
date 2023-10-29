#include "../Inc/StandardMaterial.h"

#include "../External/Imgui/imgui.h"

#include "../Inc/GraphicsCommon.h"
#include "../Inc/GraphicsContext.h"

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
		constant_buffer_ = std::make_unique<dx12::ConstantBuffer>(graphics_context->graphics_device_,
			sizeof(ConstantData), kNumBackBuffers);
	}

	void StandardMaterial::OnGui()
	{
		if(ImGui::TreeNode(GetName().c_str()))
		{
			//TODO スタンダードマテリアルのGui処理
			ImGui::SliderFloat("Metallic", &constant_data_.metallic_factor_, 0.0f, 1.0f);
			ImGui::SliderFloat("Roughness", &constant_data_.smoothness_factor_, 0.0f, 1.0f);
			ImGui::TreePop();
		}
	}

	void StandardMaterial::UpdateConstantBuffer(UINT frame_index)
	{
		constant_buffer_->CopyToGpu(&constant_data_, frame_index);
	}
}
