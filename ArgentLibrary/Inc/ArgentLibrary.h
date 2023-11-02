#pragma once
#include <string>

#include "Component/Component.h"
#include "Component/Transform.h"
#include "Component/Camera.h"
#include "Component/Light.h"

#include "Subsystem/Scene/BaseScene.h"

/**
 * \brief ���C�u������Core�@�\�܂Ƃ�
 */
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

/**
 * \brief �L�[�{�[�h�A�}�E�X�A�R���g���[�����̓��͂ɂ���
 */
namespace argent::input
{
	bool GetKey();
	bool GetKeyDown();
	bool GetKeyUp();
}

/**
 * \brief �V�[���Ǘ��ɂ���
 */
namespace argent::scene_management
{
	/**
	 * \brief �V�[���̓o�^
	 * \param scene_name �V�[���l�[��
	 * \param scene �V�[���ւ̃|�C���^
	 */
	void RegisterScene(std::string scene_name, scene::BaseScene* scene);

	/**
	 * \brief �V�[�����Z�b�g
	 * \param scene_name �V�[���l�[��
	 */
	void SetNextScene(std::string scene_name);
}
