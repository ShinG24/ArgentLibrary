#pragma once

#include <DirectXMath.h>

#include <Windows.h>

namespace argent::input
{
	enum class MouseButton
	{
		LButton,
		RButton,
		MButton,
		XButton1,
		XButton2,
		Count,
	};

	class Mouse
	{
	public:
		friend class InputManager;
		Mouse() = default;
	private:
		void Awake();
		void Update(HWND hwnd);


	public:
		bool GetButtonDown(MouseButton button) const;
		bool GetButtonUp(MouseButton button) const;
		bool GetButton(MouseButton button) const;

		DirectX::XMFLOAT2 GetPosition() const { return cur_pos_; }
		DirectX::XMFLOAT2 GetMovedVec() const { return moved_vec_; }
	private:
		bool cur_button_state_[static_cast<int>(MouseButton::Count)];
		bool pre_button_state_[static_cast<int>(MouseButton::Count)];
		DirectX::XMFLOAT2 cur_pos_;
		DirectX::XMFLOAT2 pre_pos_;
		DirectX::XMFLOAT2 moved_vec_;

		const int kMouseButtonToVKey[static_cast<int>(MouseButton::Count)]
		{
			0x01,	//LButton,
			0x02,	//RButton,
			0x04,	//MButton,
			0x05,	//XButton1,
			0x06,	//XButton2,
		};
	};
}

