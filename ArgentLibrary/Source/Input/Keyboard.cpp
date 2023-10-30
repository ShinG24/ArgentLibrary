#include "Input/Keyboard.h"

#include <crtdbg.h>
#include <Windows.h>

namespace argent::input
{
	void Keyboard::Awake()
	{
		//TODO いらないので消しましょう
		UINT key_count = sizeof(kKeyCodeToVKey) / sizeof(int);
		if (Count != key_count) _ASSERT_EXPR(FALSE, L"キーボードで入れ忘れてるやつがある");


		//すべてのキーステートをfalseで初期化
		for(unsigned int i = 0; i < sizeof(kKeyCodeToVKey) / sizeof(int); ++i)
		{
			pre_key_state_[i] = cur_key_state_[i] = false;
		}
	}

	void Keyboard::Update()
	{
		for(unsigned int i = 0; i < sizeof(kKeyCodeToVKey) / sizeof(int); ++i)
		{
			//現在のステートを前回のステートに格納
			//ステートの移動
			pre_key_state_[i] = cur_key_state_[i];

			//ステートの更新
			if(GetAsyncKeyState(kKeyCodeToVKey[i]) & 0x8000)
			{
				cur_key_state_[i] = true;
			}
			else
			{
				cur_key_state_[i] = false;
			}
		}
	}

	bool Keyboard::GetKeyDown(KeyCode key) const
	{
		return cur_key_state_[key] == true && pre_key_state_[key] == false;
	}

	bool Keyboard::GetKeyUp(KeyCode key) const 
	{
		return cur_key_state_[key] == false && pre_key_state_[key] == true;
	}

	bool Keyboard::GetKey(KeyCode key) const
	{
		return cur_key_state_[key];
	}
}
