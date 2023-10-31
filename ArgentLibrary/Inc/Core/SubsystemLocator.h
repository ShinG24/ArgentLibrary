#pragma once

#include <memory>
#include <unordered_map>

#include "Subsystem/Subsystem.h"

namespace argent
{
	

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

		template<class T>
		std::shared_ptr<T> GetSubsystem();

	private:

		void Awake();
		void Shutdown();

		template<class T>
		void AddSubsystem();

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
		if(!subsystem) _ASSERT_EXPR(false, L"Subsystemのインスタンスを作成出来ませんでした");
		subsystems_[typeid(T).hash_code()] = subsystem;
		subsystem->Awake();
	}

	template <class T>
	void SubsystemLocator::CertificateSubsystemType()
	{
		static_assert(std::is_base_of<Subsystem, T>::value, L"Subsystemではない型が指定されました");
	}
}

