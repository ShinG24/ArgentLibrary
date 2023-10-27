#include "../Inc/Material.h"

namespace argent::graphics
{
	Material::Material(const std::string& name) :
		GameResource(name)
	{}

	std::shared_ptr<Texture> Material::GetTexture(TextureUsage type)
	{
		return texture_map_.contains(type) ? texture_map_.at(type) : nullptr;
	}
}
