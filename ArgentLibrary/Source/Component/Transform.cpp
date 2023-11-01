#include "Component/Transform.h"

#include <imgui.h>

namespace argent::component
{
	Transform::Transform():
		position_(0.0f, 0.0f, 0.0f)
	,	scale_(1.0f, 1.0f, 1.0f)
	,	rotation_(0.0f, 0.0f, 0.0f)
	,	forward_(0.0f, 0.0f, 1.0f)
	,	right_(1.0f, 0.0f, 0.0f)
	,	up_(0.0f, 1.0f, 0.0f)
	{}

	void Transform::OnGui()
	{
		if(ImGui::TreeNode("Transform"))
		{
			ImGui::DragFloat3("Position", &position_.x, 0.01f, -FLT_MAX, FLT_MAX);
			ImGui::DragFloat3("Scaling", &scale_.x, 0.01f, -FLT_MAX, FLT_MAX);
			ImGui::DragFloat3("Rotation", &rotation_.x, 3.14f / 180.0f * 0.1f, -FLT_MAX, FLT_MAX);
			ImGui::TreePop();
		}
	}

	DirectX::XMFLOAT4X4 Transform::CalcWorldMatrix()
	{
		const auto s = DirectX::XMMatrixScaling(scale_.x, scale_.y, scale_.z);
		const auto r = DirectX::XMMatrixRotationRollPitchYaw(rotation_.x, rotation_.y, rotation_.z);
		const auto t = DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);
		DirectX::XMFLOAT4X4 ret{};
		DirectX::XMStoreFloat4x4(&ret, s * r * t);
		DirectX::XMStoreFloat3(&forward_, DirectX::XMVector3Normalize(r.r[2]));
		DirectX::XMStoreFloat3(&up_, DirectX::XMVector3Normalize(r.r[1]));
		DirectX::XMStoreFloat3(&right_, DirectX::XMVector3Normalize(r.r[0]));
		return ret;
	}

	void Transform::CalcDirectionalVector()
	{
		const auto r = DirectX::XMMatrixRotationRollPitchYaw(rotation_.x, rotation_.y, rotation_.z);
		DirectX::XMStoreFloat3(&forward_, DirectX::XMVector3Normalize(r.r[2]));
		DirectX::XMStoreFloat3(&up_, DirectX::XMVector3Normalize(r.r[1]));
		DirectX::XMStoreFloat3(&right_, DirectX::XMVector3Normalize(r.r[0]));
	}

	DirectX::XMFLOAT3 Transform::CalcForward()
	{
		const auto r = DirectX::XMMatrixRotationRollPitchYaw(rotation_.x, rotation_.y, rotation_.z);
		DirectX::XMStoreFloat3(&forward_, DirectX::XMVector3Normalize(r.r[2]));
		return forward_;
	}

	DirectX::XMFLOAT3 Transform::CalcUp()
	{
		const auto r = DirectX::XMMatrixRotationRollPitchYaw(rotation_.x, rotation_.y, rotation_.z);
		DirectX::XMStoreFloat3(&up_, DirectX::XMVector3Normalize(r.r[1]));
		return up_;
	}

	DirectX::XMFLOAT3 Transform::CalcRight()
	{
		const auto r = DirectX::XMMatrixRotationRollPitchYaw(rotation_.x, rotation_.y, rotation_.z);
		DirectX::XMStoreFloat3(&right_, DirectX::XMVector3Normalize(r.r[0]));
		return right_;
	}
}
