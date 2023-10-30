#include "../Inc/Camera.h"

namespace argent::component
{
	Camera::Camera() :
		focus_(0.0f, 0.0f, 1.0f)
	,	aspect_ratio_(16.0f / 9.0f)
	,	fov_y_(DirectX::XMConvertToRadians(60.0f))
	,	near_z_(0.1f)
	,	far_z_(1000.0f)
	,	auto_focus_update_(true)
	{
		transform_ = std::make_unique<Transform>();
	}

	DirectX::XMFLOAT4X4 Camera::CalcViewMatrix() const
	{
		transform_->CalcDirectionalVector();

		DirectX::XMFLOAT4X4 ret{};

		//Eyeの計算
		const auto eye = transform_->GetLocalPosition();
		auto Eye = DirectX::XMLoadFloat3(&eye);
		Eye.m128_f32[3] = 1.0f;

		//Focusの計算　Flagによって場合分け
		DirectX::XMVECTOR Focus;
		if(auto_focus_update_)
		{
			const auto forward = transform_->GetForward();
			Focus = DirectX::XMVectorAdd(Eye, DirectX::XMLoadFloat3(&forward));
		}
		else
		{
			Focus = DirectX::XMLoadFloat3(&focus_);
		}
		Focus.m128_f32[3] = 1.0f;

		const auto up = transform_->GetUp();
		const auto Up = DirectX::XMLoadFloat3(&up);

#ifdef _DEBUG
		//EyeとFocusが同じ位置ではないかチェック
		const auto equal_checker = DirectX::XMVectorEqual(Eye, Focus);
		if(equal_checker.m128_f32[0] == 0xFFFFFFFF && equal_checker.m128_f32[1] == 0xFFFFFFFF &&
			equal_checker.m128_f32[2] == 0xFFFFFFFF) _ASSERT_EXPR(false, L"Eye and Focus can not be same value");
#endif

		DirectX::XMStoreFloat4x4(&ret, DirectX::XMMatrixLookAtLH(Eye, Focus, Up));
		return ret;
	}

	DirectX::XMFLOAT4X4 Camera::CalcProjectionMatrix() const
	{
		DirectX::XMFLOAT4X4 ret{};
		DirectX::XMStoreFloat4x4(&ret, DirectX::XMMatrixPerspectiveFovLH(fov_y_, aspect_ratio_, near_z_, far_z_));
		return ret;
	}

	DirectX::XMFLOAT4X4 Camera::CalcViewProjectionMatrix() const
	{
		const auto view = CalcViewMatrix();
		const auto projection = CalcProjectionMatrix();
		DirectX::XMFLOAT4X4 ret{};
		DirectX::XMStoreFloat4x4(&ret, DirectX::XMLoadFloat4x4(&view) * DirectX::XMLoadFloat4x4(&projection));
		return ret;
	}
}
