#pragma once

#include <memory>
#include <unordered_map>

#include "Subsystem/Subsystem.h"

namespace argent
{
	/**
	 * \brief �T�u�V�X�e����ێ��A�Ǘ�����N���X
	 */
	class SubsystemLocator
	{
	public:

		friend class Engine;

	public:

		SubsystemLocator() = default;
		~SubsystemLocator() = default;

		SubsystemLocator(const SubsystemLocator&) = delete;
		SubsystemLocator(const SubsystemLocator&&) = delete;
		SubsystemLocator& operator=(const SubsystemLocator&) = delete;
		SubsystemLocator& operator=(const SubsystemLocator&&) = delete;

		/**
		 * \brief �T�u�V�X�e���̎擾
		 * \tparam T Subsystem Type
		 * \return Shared Pointer of Subsystem
		 */
		template<class T>
		std::shared_ptr<T> GetSubsystem();

	private:

		/**
		 * \brief ����������
		 * �K�v�ȃT�u�V�X�e���̃C���X�^���X�̍쐬�A�y�я������֐��̃R�[��
		 */
		void Awake();

		/**
		 * \brief �I������
		 * �K�؂ȏ��ԂŃT�u�V�X�e���̏I���������R�[������
		 */
		void Shutdown();

		/**
		 * \brief �T�u�V�X�e���̒ǉ�
		 * �e���v���[�g�����̌^��Subsystem Class���p�����Ă���K�v������
		 * \tparam T Subsystem Type
		 */
		template<class T>
		void AddSubsystem();

		/**
		 * \brief �e���v���[�g������Subsystem Class���p�����Ă��邩�m�F����
		 * \tparam T Subsystem Type
		 */
		template<class T>
		void CertificateSubsystemType();

	private:

		std::unordered_map<size_t, std::shared_ptr<Subsystem>> subsystems_;

	};

	template <class T>
	std::shared_ptr<T> SubsystemLocator::GetSubsystem()
	{
		CertificateSubsystemType<T>();
		if(!subsystems_.contains(typeid(T).hash_code())) return nullptr;
		return std::static_pointer_cast<T>(subsystems_[typeid(T).hash_code()]);
	}

	template <class T>
	void SubsystemLocator::AddSubsystem()
	{
		CertificateSubsystemType<T>();
		std::shared_ptr<T> subsystem = std::make_shared<T>();
		if(!subsystem) _ASSERT_EXPR(false, L"Subsystem�̃C���X�^���X���쐬�o���܂���ł���");
		subsystems_[typeid(T).hash_code()] = subsystem;
		subsystem->Awake();
	}

	template <class T>
	void SubsystemLocator::CertificateSubsystemType()
	{
		static_assert(std::is_base_of<Subsystem, T>::value, L"Subsystem�ł͂Ȃ��^���w�肳��܂���");
	}
}

