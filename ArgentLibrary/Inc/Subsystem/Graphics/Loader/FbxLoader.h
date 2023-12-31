#pragma once

#include <memory>

namespace argent::graphics
{
	class Model;
	/**
	 * \brief Fbxロード関数
	 * \param filepath FilePath
	 * \return モデルのShared_ptr
	 */
	std::shared_ptr<Model> LoadFbxFromFile(const char* filepath);
}