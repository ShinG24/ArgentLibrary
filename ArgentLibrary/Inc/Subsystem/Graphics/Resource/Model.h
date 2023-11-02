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

		/**
		 * \brief コンストラクタ　データはstd::move()で移行させるため所有権がなくなることに注意
		 * \param filepath Modelのパス(.exeからの相対パス) 
		 * \param mesh_vec Meshのデータ
		 * \param material_vec Materialのデータ
		 */
		Model(std::string filepath, std::vector<std::shared_ptr<Mesh>>& mesh_vec,
		      std::vector<std::shared_ptr<Material>>& material_vec);

		~Model() override = default;

		Model(const Model&) = delete;
		Model(const Model&&) = delete;
		Model& operator=(const Model&) = delete;
		Model& operator=(const Model&&) = delete;

	public:

		//TODO　この下3つの関数はユーザーには触れてほしくないのでprivateにする
		/**
		 * \brief 描画API依存のBufferオブジェクトを作成 Constにしてるけど不具合があった場合は消してもいい
		 * \param graphics_context GraphcisContextのポインタ
		 */
		void Awake(const GraphicsContext* graphics_context);

		/**
		 * \brief Guiに描画する用の関数
		 */
		void OnGui() override;

		/**
		 * \brief テクスチャなどのGPUへのアップロードが終了するまで待つ
		 */
		void WaitForUploadGpuResource();

		/**
		 * \brief .exeからの相対ファイルパス取得 
		 * \return ファイルパス
		 */
		const std::string& GetFilePath() const { return filepath_; }

		/**
		 * \brief モデルが保持しているMeshをすべて取得する関数
		 * \return Mesh
		 */
		const std::vector<std::shared_ptr<Mesh>>& GetMeshes() const { return mesh_vec_; }

		/**
		 * \brief モデルが保持しているMaterialをすべて取得する関数
		 * \return Material
		 */
		const std::vector<std::shared_ptr<Material>>& GetMaterials() const { return material_vec_; }

	private:

		std::string filepath_;	//ファイルパス
		std::vector<std::shared_ptr<Mesh>> mesh_vec_;
		std::vector<std::shared_ptr<Material>> material_vec_;

		bool is_awake_ = false;
		//TODO アニメーションに対応
	};
}