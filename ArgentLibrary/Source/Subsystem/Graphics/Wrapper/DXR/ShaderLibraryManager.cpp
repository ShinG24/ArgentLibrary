#include "Subsystem/Graphics/Wrapper/DXR/ShaderLibraryManager.h"

#include "Subsystem/Graphics/Wrapper/DXR/ShaderLibrary.h"


namespace argent::graphics
{
	void ShaderLibraryManager::AddShaderLibrary(const char* filepath,
		const std::vector<std::wstring>& export_name)
	{
		shader_libraries_.emplace_back(std::make_unique<ShaderLibrary>(filepath, 
			export_name));
	}
	
}
