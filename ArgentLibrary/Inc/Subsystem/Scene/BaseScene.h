#pragma once

#include <string>

namespace argent::scene
{
	/**
	 * \brief �V�[�����N���X
	 */
	class BaseScene
	{
	public:

		BaseScene() = default;
		virtual ~BaseScene() = default;

		BaseScene(const BaseScene&) = delete;
		BaseScene(const BaseScene&&) = delete;
		BaseScene& operator=(const BaseScene&) = delete;
		BaseScene& operator=(const BaseScene&&) = delete;

		/**
		 * \brief �������֐�
		 * ���̃V�[����Current Scene�Ƃ��Đݒ肳��čŏ��ɌĂ΂��
		 * ���̊֐��͏�L�̏����𖞂������тɌĂ΂��
		 */
		virtual void Awake()
		{
			int i = 0;
		}

		/**
		 * \brief �I���֐�
		 * ���̃V�[����Curren Scene�ł͂Ȃ��Ȃ����Ƃ��ɍŌ�ɌĂ΂��
		 * ���̊֐��͏�L�̏����𖞂������тɌĂ΂��
		 */
		virtual void Shutdown() {}

		/**
		 * \brief �X�V�֐�
		 * ���̃V�[����Current Scene�ł������A���t���[�����Ă΂��
		 * ���̊֐��͕K��Render�����O�ŌĂ΂��
		 */
		virtual void Update() {}

		/**
		 * \brief �`��֐�
		 * ���̃V�[����Current Scene�ł�����薈�t���[�����Ă΂��
		 * ���̊֐���Update()����ɌĂ΂�AOnGui()����ɌĂ΂��
		 */
		virtual	void Render() {}

		/**
		 * \brief Gui��ɕ`�悷��֐�
		 */
		virtual void OnGui() {}

		std::string GetName() const { return name_; }

	protected:

		std::string name_ ;
	};
}

