#include "Subsystem/Graphics/Wrapper/DXR/ShaderLibrary.h"

#include <filesystem>

#include "Subsystem/Graphics/Wrapper/ShaderCompiler.h"

namespace argent::graphics
{
	ShaderLibrary::ShaderLibrary(const char* filepath, std::vector<LibraryData>& library_data):
		library_data_(std::move(library_data))
	,	filepath_(filepath)
	{
		ShaderCompiler compiler;
		std::filesystem::path path = filepath;

		compiler.CompileShaderLibrary(path.wstring().c_str(), library_object_.ReleaseAndGetAddressOf());
	}
}
