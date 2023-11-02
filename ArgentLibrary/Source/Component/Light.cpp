#include "Component/Light.h"

#include <imgui.h>

argent::component::Light::Light()
{

}

void argent::component::Light::OnGui()
{
	if(ImGui::TreeNode("Light"))
	{
		ImGui::DragFloat3("Direction", &direction_.x);
		ImGui::TreePop();
	}
}
