#include "../Inc/Model.h"

#include <filesystem>

#include "GraphicsDevice.h"
#include "GraphicsContext.h"

#include "Mesh.h"
#include "Material.h"

namespace argent::graphics
{
	//ファイルネームの抽出
	inline const char* Stem(const std::string& value)
	{
		std::filesystem::path path = value;
		return path.stem().string().c_str();
	}

	Model::Model(std::string& filepath, std::vector<std::shared_ptr<Mesh>>& mesh_vec,
		std::vector<std::shared_ptr<Material>>& material_vec) :
		GameResource(Stem(filepath))
	,	filepath_(std::move(filepath))
	,	mesh_vec_(std::move(mesh_vec))
	,	material_vec_(std::move(material_vec))
	{}

	void Model::Awake(const GraphicsContext* graphics_context)
	{
		for (auto& m : mesh_vec_)
		{
			m->Awake(graphics_context);
		}

		for(auto& m : material_vec_)
		{
			m->Awake(graphics_context);
		}
	}
}
