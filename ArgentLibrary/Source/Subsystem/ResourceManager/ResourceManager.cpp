#include "Subsystem/ResourceManager/ResourceManager.h"


namespace argent
{
	uint64_t ResourceManager::GenerateUniqueId()
	{
		static uint64_t unique_id = 0;
		return unique_id++;
	}
}

