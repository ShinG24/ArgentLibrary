#pragma once

#include <memory>

namespace argent::game_resource
{
	class Model;

	std::shared_ptr<Model> LoadFbx(const char* filename);	
}