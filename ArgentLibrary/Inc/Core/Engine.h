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
		void Initialize(long window_width, long window_height) const;

		/**
		 * \brief �I������
		 * �e�T�u�V�X�e���̏I��������Call
		 */
		void Finalize() const;

		/**
		 * \brief �A�v���P�[�V�����̎��s
		 */
		void Run();

		SubsystemLocator* GetSubsystemLocator() const { return subsystem_locator_.get(); }

		long GetWindowWidth() const { return window_width_; }
		long GetWindowHeight() const { return window_height_; }

	private:

		std::unique_ptr<SubsystemLocator> subsystem_locator_;
		long window_width_;
		long window_height_;

	};


	/**
	 * \brief ���N���X����̃A�N�Z�X�p
	 * ���C�u�����쐬�҂݂̂��G���悤�ɂ��邽�߁AArgentLibrary.h�ɂ̓C���N���[�h���Ȃ�����
	 * \return Engine Pointer
	 */
	Engine* GetEngine();

}

