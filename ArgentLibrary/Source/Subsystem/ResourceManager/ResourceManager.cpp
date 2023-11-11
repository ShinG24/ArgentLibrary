#include "Subsystem/ResourceManager/ResourceManager.h"


namespace argent::resource_system
{
	uint64_t ResourceManager::unique_id_ = 0u;

	void ResourceManager::Awake()
	{
		resources_.clear();
	}

	void ResourceManager::Shutdown()
	{
		resources_.clear();
	}

	uint64_t ResourceManager::Register(const std::shared_ptr<graphics::GameResource>& resource)
	{
		const auto unique_id = GenerateUniqueId();
		resources_[unique_id] = resource;
		return unique_id;
	}

	bool ResourceManager::HasResource(const std::string& name) const
	{
		for(const auto& r : resources_)
		{
			if(r.second->GetName() == name)
			{
				return true;
			}
		}
		return false;
	}

	uint64_t ResourceManager::GenerateUniqueId()
	{
		return unique_id_++;
	}
}

