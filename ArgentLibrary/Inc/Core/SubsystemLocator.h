#pragma once

#include <memory>
#include <unordered_map>

#include "Subsystem/Subsystem.h"

namespace argent
{
	/**
	 * \brief サブシステムを保持、管理するクラス
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
		 * \brief サブシステムの取得
		 * \tparam T Subsystem Type
		 * \return Shared Pointer of Subsystem
		 */
		template<class T>
		std::shared_ptr<T> GetSubsystem();

	private:

		/**
		 * \brief 初期化処理
		 * 必要なサブシステムのインスタンスの作成、及び初期化関数のコール
		 */
		void Awake();

		/**
		 * \brief 終了処理
		 * 適切な順番でサブシステムの終了処理をコールする
		 */
		void Shutdown();

		/**
		 * \brief サブシステムの追加
		 * テンプレート引数の型はSubsystem Classを継承している必要がある
		 * \tparam T Subsystem Type
		 */
		template<class T>
		void AddSubsystem();

		/**
		 * \brief テンプレート引数がSubsystem Classを継承しているか確認する
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

