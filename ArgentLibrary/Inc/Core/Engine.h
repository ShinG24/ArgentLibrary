#pragma once

#include <memory>

namespace argent
{
	class SubsystemLocator;

	/**
	 * \brief Core Class
	 * ���ׂẴT�u�V�X�e���𓝊����A
	 * �T�u�V�X�e���̏������A���C�����[�v�A�I���Ȃǂ̏������s��
	 */
	class Engine
	{
	public:

		Engine();
		~Engine() = default;

		Engine(const Engine&) = delete;
		Engine(const Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine& operator=(const Engine&&) = delete;

		/**
		 * \brief �����������@
		 * �e�T�u�V�X�e���̃C���X�^���X���A�y�я�����
		 */
		void Initialize();

		/**
		 * \brief �I������
		 * �e�T�u�V�X�e���̏I��������Call
		 */
		void Finalize();

		/**
		 * \brief �A�v���P�[�V�����̎��s
		 */
		void Run();

	private:

		std::unique_ptr<SubsystemLocator> subsystem_locator_;

	};


	/**
	 * \brief ���N���X����̃A�N�Z�X�p
	 * ���C�u�����쐬�҂݂̂��G���悤�ɂ��邽�߁AArgentLibrary.h�ɂ̓C���N���[�h���Ȃ�����
	 * \return Engine Pointer
	 */
	Engine* GetEngine();
}

