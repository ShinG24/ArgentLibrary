#include "../Inc/Material.h"

#include "../Inc/Texture.h"

namespace argent::graphics
{
	Material::Material(const std::string& name) :
		GameResource(name)
	{}

	void Material::WaitForGpu()
	{
		for(const auto& t : texture_map_)
		{
			t.second->WaitBeforeUse();
		}
	}

	std::shared_ptr<Texture> Material::GetTexture(TextureUsage type)
	{
		return texture_map_.contains(type) ? texture_map_.at(type) : nullptr;
	}
}
