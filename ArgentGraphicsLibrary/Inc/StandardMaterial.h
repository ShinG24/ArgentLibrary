#pragma once

#include "Material.h"

namespace argent::graphics
{
	/**
	 * \brief デフォルトの3DMeshマテリアル PBRマテリアル
	 * FbxLoaderからモデルをインポートした場合、初期のマテリアルはこれ
	 * メタリック、ラフネス強度をメンバ変数として持つ
	 * 使用可能なテクスチャタイプはAlbedo, Normal, Metllic, Roughnessの4つ
	 * //TODO Height, Maskにも対応する
	 */
	class StandardMaterial final : public Material
	{
	public:

		//インスタンス作成用のデータ
		struct Data
		{
			std::unordered_map<TextureUsage, std::string> filepath_map_;
			float metallic_factor_;
			float smoothness_factor_;
		};

		//コンスタントバッファ用のデータ
		struct ConstantData
		{
			//TODO Colorってあったほうがいい？？
			float metallic_factor_ = 0.0f;
			float smoothness_factor_ = 0.5f;
		};

	public:

		StandardMaterial() = default;
		StandardMaterial(const std::string& name, const Data& data);

		~StandardMaterial() override = default;

		StandardMaterial(const StandardMaterial&) = delete;
		StandardMaterial(const StandardMaterial&&) = delete;
		StandardMaterial &operator=(const StandardMaterial&) = delete;
		StandardMaterial &operator=(const StandardMaterial&&) = delete;

	public:

		/**
		 * \brief 描画API依存のBuffer or Texture を作成
		 * \param graphics_context GraphicsContext
		 */
		void Awake(const GraphicsContext* graphics_context) override;

		/**
		 * \brief Gui上に描画
		 */
		void OnGui() override;

	private:

		Data data_;	//TODO こいつは保持しておく必要があるのか？
		ConstantData constant_data_;
	};

}