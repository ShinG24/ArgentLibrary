#pragma once

#include <memory>

#include "../Subsystem.h"

namespace argent
{
	/**
	 * \brief リソース管理用のクラス
	 */
	class ResourceManager final : public Subsystem
	{

	public:

		ResourceManager();
		~ResourceManager() override = default;

		ResourceManager(const ResourceManager&) = delete;
		ResourceManager(const ResourceManager&&) = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;
		ResourceManager& operator=(const ResourceManager&&) = delete;

		void Awake() override;
		void Shutdown() override;


	private:
		static uint64_t GenerateUniqueId();

	};
}

