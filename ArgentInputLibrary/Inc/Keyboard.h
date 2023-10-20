// ReSharper disable All
#pragma once

namespace argent::input
{
	enum KeyCode
	{
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,

		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,

		Num1,
		Num2,
		Num3,
		Num4,
		Num5,
		Num6,
		Num7,
		Num8,
		Num9,
		Num0,

		LShift,
		RShift,
		LCtrl,
		RCtrl,
		LAlt,
		RAlt,
		Tab,
		CapsLock,
		Delete,

		Escape,
		Space,

		Count,
	};


	class Keyboard
	{
		friend class InputManager;
	public:
		Keyboard() = default;

	private:

		void Awake();
		void Update();

	public:
		/**
		 * \brief 押された瞬間かどうか
		 * \param key キーコード
		 * \return 
		 */
		bool GetKeyDown(KeyCode key) const;

		/**
		 * \brief 離した瞬間かどうか
		 * \param key キーコード
		 * \return 
		 */
		bool GetKeyUp(KeyCode key) const;

		/**
		 * \brief 押されているかどうか
		 * \param key キーコード
		 * \return 
		 */
		bool GetKey(KeyCode key) const;

	private:

		bool cur_key_state_[KeyCode::Count];
		bool pre_key_state_[KeyCode::Count];

		//自作のキーコードをwindows virtual keycode へ変換する
		const int kKeyCodeToVKey[KeyCode::Count]
		{
			0x41,// A,
			0x42,// B,
			0x43,// C,
			0x44,// D,
			0x45,// E,
			0x46,// F,
			0x47,// G,
			0x48,// H,
			0x49,// I,
			0x4A,// J,
			0x4B,// K,
			0x4C,// L,
			0x4D,// M,
			0x4E,// N,
			0x4F,// O,
			0x50,// P,
			0x51,// Q,
			0x52,// R,
			0x53,// S,
			0x54,// T,
			0x55,// U,
			0x56,// V,
			0x57,// W,
			0x58,// X,
			0x59,// Y,
			0x5A,// Z,
			// 
			0x70,// F1,
			0x71,// F2,
			0x72,// F3,
			0x73,// F4,
			0x74,// F5,
			0x75,// F6,
			0x76,// F7,
			0x77,// F8,
			0x78,// F9,
			0x79,// F10,
			0x7A,// F11,
			0x7B,// F12,
			// 
			0x31,// Num1,
			0x32,// Num2,
			0x33,// Num3,
			0x34,// Num4,
			0x35,// Num5,
			0x36,// Num6,
			0x37,// Num7,
			0x38,// Num8,
			0x39,// Num9,
			0x30,// Num0,

			0xA0,//LShift,
			0xA1,//RShift,
			0xA2,//LCtrl,
			0xA3,//RCtrl,
			0xA4,//LAlt,
			0xA5,//RAlt,

			0x09,//Tab,
			0x14,//CapsLock,
			0x2E,//Delete,

			0x1B,//Escapce
			0x20,//Space
		};
	};
}

