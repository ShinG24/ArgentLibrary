#pragma once

#include <memory>
#include <string>
#include <vector>

namespace argent::graphics
{
	class ShaderLibrary;

	class ShaderLibraryManager
	{
	public:

		ShaderLibraryManager() = default;
		~ShaderLibraryManager() = default;

		ShaderLibraryManager(const ShaderLibraryManager&) = delete;
		ShaderLibraryManager(const ShaderLibraryManager&&) = delete;
		ShaderLibraryManager& operator=(const ShaderLibraryManager&) = delete;
		ShaderLibraryManager& operator=(const ShaderLibraryManager&&) = delete;

		void AddShaderLibrary(const char* filepath, const std::vector<std::wstring>& export_name);

	private:

		std::vector<std::unique_ptr<ShaderLibrary>> shader_libraries_;

	};
}

