#pragma once

#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

#include "../Subsystem.h"
#include "Subsystem/Graphics/Resource/GameResource.h"


namespace argent::graphics
{
	class GameResource;
}

namespace argent::resource_system
{
	/**
	 * \brief ���\�[�X�Ǘ��p�̃N���X
	 */
	class ResourceManager final : public Subsystem
	{

	public:

		ResourceManager() = default;
		~ResourceManager() override = default;

		ResourceManager(const ResourceManager&) = delete;
		ResourceManager(const ResourceManager&&) = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;
		ResourceManager& operator=(const ResourceManager&&) = delete;

		void Awake() override;
		void Shutdown() override;

		/**
		 * \brief Unique ID�ɂ�郊�\�[�X�̎擾
		 * \tparam T �^��
		 * \param unique_id Unique ID 
		 * \return ������Ȃ������ꍇ��Nullptr
		 */
		template<class T>
		std::shared_ptr<T> GetResource(uint64_t unique_id)
		{
			if(resources_.contains(unique_id))
			{
				//if (typeid(*(resources_[unique_id].get())) != typeid(T) || std::is_base_of_v<graphics::GameResource, T>) _ASSERT_EXPR(false, L"�w�肳�ꂽ�^���ƕێ�����Ă���^�����Ⴂ�܂�");
				return std::static_pointer_cast<T>(resources_[unique_id]);
			}
			return nullptr;
		}

		/**
		 * \brief ���O�ɂ�郊�\�[�X�̎擾 : �������O���������ꍇ�͂��܂������Ȃ��\��������܂�
		 * \tparam T �^��
		 * \param name Name
		 * \return ������Ȃ������ꍇ��Nullptr
		 */
		template<class T>
		std::shared_ptr<T> GetResource(const std::string& name)
		{
			for (const auto& r : resources_)
			{
				if (r.second->GetName() == name) return std::static_pointer_cast<T>(r.second);
			}
			return nullptr;
		}

		/**
		 * \brief ���\�[�X�̓o�^
		 * \param resource GameResource
		 * \return Unique ID
		 */
		uint64_t Register(const std::shared_ptr<graphics::GameResource>& resource);


		bool HasResource(uint64_t unique_id) const { return resources_.contains(unique_id); }
		bool HasResource(const std::string& name) const;

		
	private:

		static uint64_t GenerateUniqueId();
		static uint64_t unique_id_;

	private:

		std::unordered_map<uint64_t, std::shared_ptr<graphics::GameResource>> resources_;
	};
}
