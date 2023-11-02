#pragma once

#include <memory>

namespace argent::graphics
{
	class Model;
	/**
	 * \brief Fbxƒ[ƒhŠÖ”
	 * \param filepath FilePath
	 * \return ƒ‚ƒfƒ‹‚ÌShared_ptr
	 */
	std::shared_ptr<Model> LoadFbxFromFile(const char* filepath);
}