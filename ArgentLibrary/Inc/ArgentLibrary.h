#pragma once
#include <string>

namespace argent
{
	/**
	 * \brief Argent Library������
	 * ���̊֐���Call����̂�1�񂾂��ŗǂ�
	 * �`��A���o�́A�^�C�}�[���̃��C�u�����T�u�V�X�e���̏�����������
	 * \param window_width  : �E�B���h�E�̕�
	 * \param window_height : �E�B���h�E�̍���
	 */
	void Initialize(long window_width, long window_height);

	/**
	 * \brief Argent Library�I������
	 * ���̊֐��̓v���O�������I������Ƃ���1�x�����Ă�
	 *
	 */
	void Finalize();

	/**
	 * \brief ���C�u�������C�����[�v
	 * �Q�[�����[�v�����͂����ōs����
	 */
	void AppRun();
}

namespace argent::input
{
	bool GetKey();
	bool GetKeyDown();
	bool GetKeyUp();
}

namespace argent::scene
{
	class BaseScene;
}
namespace argent::scene_management
{
	void RegisterScene(std::string scene_name, scene::BaseScene* scene);
	void SetNextScene(std::string scene_name);
}