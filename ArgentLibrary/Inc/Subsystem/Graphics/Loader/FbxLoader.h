#pragma once

#include <memory>

namespace argent::graphics
{
	class Model;
	/**
	 * \brief Fbx���[�h�֐�
	 * \param filepath FilePath
	 * \return ���f����Shared_ptr
	 */
	std::shared_ptr<Model> LoadFbxFromFile(const char* filepath);
}