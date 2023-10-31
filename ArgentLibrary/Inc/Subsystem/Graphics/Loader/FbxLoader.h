#pragma once

#include <memory>

namespace argent::graphics
{
	class Model;
	std::shared_ptr<Model> LoadFbxFromFile(const char* filepath);
}

namespace argent::game_resource
{
	//TODO こっち側のローダーは上側がレイトレに対応し次第消す
	class Model;

	std::shared_ptr<Model> LoadFbx(const char* filename);	
}