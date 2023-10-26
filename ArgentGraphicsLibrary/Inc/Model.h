#pragma once

#include <vector>
#include <memory>

#include "GameResource.h"

namespace argent::graphics
{
	struct GraphicsContext;
}

namespace argent::graphics
{
	class Mesh;
	class Material;

	/**
	 * \brief モデルクラス
	 * MeshとMaterialを複数個保持しており、オプションでBoneとAnimationを保持する
	 * MeshとMaterialは一個以上保持していることが保証されている(持っていない場合はAssertionが呼ばれる)
	 * アニメーションする場合のみBoneとAnimationを保持する
	 */
	class Model final : GameResource
	{
	public:
		Model() = default;
		~Model() override = default;

		/**
		 * \brief コンストラクタ　データはstd::move()で移行させるため所有権がなくなることに注意
		 * \param filepath Modelのパス(.exeからの相対パス) 
		 * \param mesh_vec Meshのデータ
		 * \param material_vec Materialのデータ
		 */
		Model(std::string& filepath, std::vector<std::shared_ptr<Mesh>>& mesh_vec,
		      std::vector<std::shared_ptr<Material>>& material_vec);

		void Awake(const GraphicsContext* graphics_context);

		Model(const Model&) = delete;
		Model(const Model&&) = delete;
		Model& operator=(const Model&) = delete;
		Model& operator=(const Model&&) = delete;

		/**
		 * \brief Guiに描画する用の関数
		 */
		void OnGui() override;

	private:
		std::string filepath_;	//ファイルパス
		std::vector<std::shared_ptr<Mesh>> mesh_vec_;
		std::vector<std::shared_ptr<Material>> material_vec_;
	};
}