#pragma once

#include <memory>
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

		void AddShaderLibrary();
	private:

		std::vector<std::unique_ptr<ShaderLibrary>> shader_libraries_;

	};
}

