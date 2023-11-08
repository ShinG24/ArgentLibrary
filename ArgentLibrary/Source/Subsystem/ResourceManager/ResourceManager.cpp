#include "Subsystem/ResourceManager/ResourceManager.h"


namespace argent
{
	uint64_t ResourceManager::unique_id_ = 0u;

	uint64_t ResourceManager::GenerateUniqueId()
	{
		return unique_id_++;
	}
}

