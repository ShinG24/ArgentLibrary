#include "Scene.h"

#include <iostream>

#include "ArgentLibrary.h"

#include "ArgentLibrary/External/Imgui/imgui.h"

void DemoScene::Awake()
{
	std::cout << "awake";
}

void DemoScene::Shutdown()
{

}

void DemoScene::Update()
{
	if (!argent::input::GetButton(argent::input::MouseButton::RButton)/*�E�N���b�N�̓���*/) return;
	
	//�J�����̈ړ�
	
	//�������Z�o
	camera_.GetTransform()->CalcDirectionalVector();
	auto forward = camera_.GetTransform()->GetForward();
	auto right = camera_.GetTransform()->GetRight();
	const DirectX::XMVECTOR F = DirectX::XMLoadFloat3(&forward);
	const DirectX::XMVECTOR R = DirectX::XMLoadFloat3(&right);
	const DirectX::XMVECTOR U = { 0.0f, 1.0f, 0.0f, 0.0f };

	float front_input = 0;
	float right_input = 0;
	float up_input = 0;

	//�O��
	if (argent::input::GetKey(argent::input::W)/* Keyboard W */) front_input += 1.0f;
	if (argent::input::GetKey(argent::input::S)/* Keyboard S */) front_input -= 1.0f;

	//���E
	if (argent::input::GetKey(argent::input::A)/* Keyboard A */) right_input -= 1.0f;
	if (argent::input::GetKey(argent::input::D)/* Keyboard D */) right_input += 1.0f;

	//�㉺
	if (argent::input::GetKey(argent::input::Q)/* Keyboard Q */) up_input -= 1.0f;
	if (argent::input::GetKey(argent::input::E)/* Keyboard E */) up_input += 1.0f;

	using namespace DirectX;

	const DirectX::XMVECTOR move_vector = DirectX::XMVector3Normalize(F * front_input + U * up_input + R * right_input);
	DirectX::XMFLOAT3 p{};
	DirectX::XMStoreFloat3(&p,  move_vector * move_speed_);
	camera_.GetTransform()->AddPosition(p);

	//�J�����̉�]
	{
		const float dx = argent::input::GetMouseMoveX();	//�}�E�XX�������̈ړ��l
		const float dy = argent::input::GetMouseMoveY();	//�}�E�XY�������̈ړ��l

		const float x_d_angle = dy * rotation_speed_;
		const float y_d_angle = dx * rotation_speed_;

		DirectX::XMFLOAT3 r{ x_d_angle, y_d_angle, 0.0f };
		camera_.GetTransform()->AddRotation(r);
	}
}

void DemoScene::Render()
{

}

void DemoScene::OnGui()
{
	if(ImGui::TreeNode("ImGui"))
	{
		ImGui::DragFloat("Move Speed", &move_speed_, 0.001f, 0.0f, 10.0f);
		ImGui::DragFloat("Rotation Speed", &rotation_speed_, 0.001f, 0.0f, 10.0f);
		camera_.OnGui();
		directional_light_.OnGui();
		ImGui::TreePop();
	}
}
