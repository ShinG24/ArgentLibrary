#pragma once
#include "GameResource.h"

#include <memory>
#include <unordered_map>

#include "Texture.h"

namespace argent::graphics::dx12
{
	struct Descriptor;
	class ConstantBuffer;
}

namespace argent::graphics
{
	struct GraphicsContext;
	class Texture;
}

namespace argent::graphics
{
	class GameResource;

	/**
	 * \brief マテリアル基底クラス
	 * 何をもたせればいいかわからないのでほぼ何も持たせていない
	 */
	class Material : public GameResource
	{
	public:

		enum class TextureUsage
		{
			Albedo,		//Albedo (Base Color)	1番基本な色の出力を担う　デフォルトは真っ白なテクスチャ
						//Diffuseテクスチャはこれに該当する前提で組んでます
			Normal,		//Normal Map 接線空間の法線情報を格納したテクスチャ　法線の計算に使う
			Metallic,	//Metallic	金属感を表すためのテクスチャ　デフォルトは多分黒のテクスチャ
			Roughness,	//Roughness 表面の粗さを表現するためのテクスチャ デフォルトは多分黒のテクスチャ
			Emissive,	//Emissive	輝度調節用？のテクスチャ　輝きが増えます
			Height,		//Height Map 高さとかに使うらしい　実装は出来てない
			Mask,		//Mask		多分色々使えるけど知らない
		};

	public:

		Material() = default;

		/**
		 * \brief コンストラクタ
		 * \param name Material name
		 */
		Material(const std::string& name);
		~Material() override = default;

		Material(const Material&) = delete;
		Material(const Material&&) = delete;
		Material& operator=(const Material&) = delete;
		Material& operator=(const Material&&) = delete;

	public:

		/**
		 * \brief テクスチャあるいはコンスタントバッファの初期化関数
		 * マテリアルは可能な限り薄い構造にするため、ここでは何もしないようにする
		 * \param graphics_context GraphicsContext
		 */
		virtual void Awake(const GraphicsContext* graphics_context) = 0;

		/**
		 * \brief テクスチャのアップロードが終了するまで待つ
		 */
		virtual void WaitForGpu();

		/**
		 * \brief　コンスタントバッファのUpdate
		 * \param frame_index フレームインデックス
		 */
		virtual void UpdateConstantBuffer(UINT frame_index) = 0;

		/**
		 * \brief マテリアルインスタンスでのテクスチャの使い道から該当のテクスチャを取得する
		 * もし持っていなかった場合はNullptrが変える
		 * \param type インスタンスでのテクスチャの使用用途
		 * \return Textureへのポインタ
		 */
		std::shared_ptr<Texture> GetTexture(TextureUsage type);

		/**
		 * \brief テクスチャのGPUハンドルを取得する
		 * //TODO この関数はマテリアルが保持しているテクスチャが同じディスクリプタヒープ上に存在し、
		 * かつ連続している前提で組んでいるので変更する
		 * \return d3d12 Gpu Descriptor handle
		 */
		D3D12_GPU_DESCRIPTOR_HANDLE GetTextureGpuHandleBegin() const { return texture_map_.begin()->second->GetGpuHandle(); }

		/**
		 * \brief コンスタントバッファのGpuアドレスを取得
		 * \param frame_index フレームインデックス
		 * \return gpu address
		 */
		D3D12_GPU_VIRTUAL_ADDRESS GetConstantGpuVirtualAddress(UINT frame_index) const;

		/**
		 * \brief コンスタントバッファのDescriptorHandleを取得
		 * \param frame_index フレームインデックス
		 * \return descriptor
		 */
		dx12::Descriptor GetConstantDescriptorHandle(UINT frame_index) const;

		//継承前提の組み方で継承先からアクセスする可能性が大いにあるため、処理速度を考えGetterではなくprotectedにしてアクセスできるようにする
	protected:
		//TODO テクスチャを持たないマテリアルは存在するのか あったとしてもテクスチャマップを使わなければいいだけなので
		//とりあえずおいておきます
		std::unordered_map<TextureUsage, std::shared_ptr<Texture>> texture_map_;	//Texture map　2つ以上の使用用途が被ったテクスチャは保持できない 
		std::unique_ptr<dx12::ConstantBuffer> constant_buffer_;
	};
}
