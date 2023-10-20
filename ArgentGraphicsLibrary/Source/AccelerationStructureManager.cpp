#include "../Inc/AccelerationStructureManager.h"

#include "../Inc/GraphicsDevice.h"

namespace argent::graphics::dxr
{
	UINT AccelerationStructureManager::AddBottomLevelAS(GraphicsDevice* graphics_device, 
		GraphicsCommandList* graphics_command_list, BLASBuildDesc* build_desc)
	{
		auto b = std::make_unique<BottomLevelAccelerationStructure>(graphics_device, graphics_command_list, 
			build_desc);
	}

	void AccelerationStructureManager::GetBottomLevelAS(UINT unique_id)
	{

	}

	UINT AccelerationStructureManager::GenerateUniqueID()
	{
		static UINT unique_id = 0;
		return unique_id++;
	}
}
