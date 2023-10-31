#include "Subsystem/Input/Mouse.h"

namespace argent::input
{
	void Mouse::Awake()
	{
		for (int i = 0; i < static_cast<int>(MouseButton::Count); ++i)
		{
			pre_button_state_[i] = cur_button_state_[i] = false;
		}
		cur_pos_ = pre_pos_ = moved_vec_ = DirectX::XMFLOAT2();
	}

	void Mouse::Update(HWND hwnd)
	{
		for(int i = 0; i < static_cast<int>(MouseButton::Count); ++i)
		{
			pre_button_state_[i] = cur_button_state_[i];

			cur_button_state_[i] = (GetAsyncKeyState(kMouseButtonToVKey[i]) & 0x8000);
		}

		pre_pos_ = cur_pos_;

		POINT point{};
		GetCursorPos(&point);
		ScreenToClient(hwnd, &point);

		cur_pos_ = DirectX::XMFLOAT2(static_cast<float>(point.x), static_cast<float>(point.y));

		moved_vec_ = DirectX::XMFLOAT2(cur_pos_.x - pre_pos_.x, cur_pos_.y - pre_pos_.y);
	}

	bool Mouse::GetButtonDown(MouseButton button) const
	{
		return cur_button_state_[static_cast<int>(button)] == true && pre_button_state_[static_cast<int>(button)] == false;
	}

	bool Mouse::GetButtonUp(MouseButton button) const
	{
		return cur_button_state_[static_cast<int>(button)] == false && pre_button_state_[static_cast<int>(button)] == true;
	}

	bool Mouse::GetButton(MouseButton button) const
	{
		return cur_button_state_[static_cast<int>(button)];
	}
}
