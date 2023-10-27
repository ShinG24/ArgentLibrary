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
	class StandardMaterial final : Material
	{
	public:

		//インスタンス作成用のデータ
		struct Data
		{
			std::string name_;
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
		StandardMaterial(const Data& data);

		~StandardMaterial() override = default;

		void Awake(const GraphicsContext* graphics_context) override;
		void OnGui() override;
	private:
		Data data_;	//TODO こいつは保持しておく必要があるのか？
		ConstantData constant_data_;
	};

}