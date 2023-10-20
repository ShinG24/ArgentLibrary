#include "../Inc/Keyboard.h"

#include <crtdbg.h>
#include <Windows.h>

namespace argent::input
{
	void Keyboard::Awake()
	{
		//TODO ����Ȃ��̂ŏ����܂��傤
		UINT key_count = sizeof(kKeyCodeToVKey) / sizeof(int);
		if (Count != key_count) _ASSERT_EXPR(FALSE, L"�L�[�{�[�h�œ���Y��Ă�������");


		//���ׂẴL�[�X�e�[�g��false�ŏ�����
		for(unsigned int i = 0; i < sizeof(kKeyCodeToVKey) / sizeof(int); ++i)
		{
			pre_key_state_[i] = cur_key_state_[i] = false;
		}
	}

	void Keyboard::Update()
	{
		for(unsigned int i = 0; i < sizeof(kKeyCodeToVKey) / sizeof(int); ++i)
		{
			//���݂̃X�e�[�g��O��̃X�e�[�g�Ɋi�[
			//�X�e�[�g�̈ړ�
			pre_key_state_[i] = cur_key_state_[i];

			//�X�e�[�g�̍X�V
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
