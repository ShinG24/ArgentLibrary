#pragma once

#include <DirectXMath.h>

#include <memory>
#include <vector>

#include "GameResource.h"

namespace argent::graphics::dx12
{
	class VertexBuffer;
	class IndexBuffer;
	//class BottomLevelAccelerationStructure;
}

namespace argent::graphics
{
	struct GraphicsContext;
	class Material;

	using Position	= DirectX::XMFLOAT3;
	using Normal	= DirectX::XMFLOAT3;
	using Tangent	= DirectX::XMFLOAT3;
	using Binormal	= DirectX::XMFLOAT3;
	using Texcoord	= DirectX::XMFLOAT2;

	/**
	 * \brief 3DMeshのデータ保持クラス
	 * 描画用の生データ、及び描画APIに依存したBufferの他に
	 * 当たり判定用の生データを持つ
	 * 描画用の生データ、Bufferは必ず保持しており、当たり判定用のデータはオプション
	 */
	class Mesh final : GameResource
	{
	public:

		friend class Model;

		struct Data
		{
			/**
			 * \brief Serialize専用のコンストラクタ 
			 */
			Data() = default;

			/**
			 * \brief コンストラクタ 
			 * \param position_vec : position vec
			 * \param normal_vec : normal vec
			 * \param tangent_vec : tangent vec
			 * \param binormal_vec : binormal vec
			 * \param texcoord_vec : texcoord vec
			 * \param index_vec : index vec
			 * \param default_global_transform	: default global transform
			 * \param material : material //TODO そのうちけします 
			 */
			Data(std::vector<Position>& position_vec, std::vector<Normal> normal_vec, std::vector<Tangent>& tangent_vec, 
			     std::vector<Binormal>& binormal_vec, std::vector<Texcoord>& texcoord_vec, 
			     std::vector<uint32_t>& index_vec, const DirectX::XMFLOAT4X4& default_global_transform, std::shared_ptr<Material> material);

			std::vector<Position> position_vec_{};
			std::vector<Normal> normal_vec_{};
			std::vector<Tangent> tangent_vec_{};
			std::vector<Binormal> binormal_vec_{};
			std::vector<Texcoord> texcoord_vec_{};
			std::vector<uint32_t> index_vec_{};

			DirectX::XMFLOAT4X4 default_global_transform_{};	//グローバル空間でのトランスフォーム　
																//Global Space = Maya, Blenderでいうシーン World ↔ Local(Model) ↔ Global
			std::shared_ptr<Material> material_;

		private:

			friend class Mesh;

			/**
			 * \brief .emptyなメンバがないかの確認用
			 * ライブラリとしてMeshはすべてのメンバを持っているという前提で組んでいるため
			 * 持っていない場合はアサーションを呼ぶようにしている
			 * \return true : .emptyなデータがある  false : ない
			 */
			bool HasNullData() const;
		};

	public:

		Mesh() = default;
		Mesh(const std::string& name, const Data& mesh_data):
			GameResource(name)
		,	data_(mesh_data)
		,	collision_data_(mesh_data)
		,	blas_unique_id_(~0u)
		{}

		Mesh(const std::string& name, const Data& rendering_mesh_data, const Data& collision_mesh_data):
			GameResource(name)
		,	data_(rendering_mesh_data)
		,	collision_data_(collision_mesh_data)
		,	blas_unique_id_(~0u)
		{}

		~Mesh() override = default;

		Mesh(const Mesh&) = delete;
		Mesh(const Mesh&&) = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(const Mesh&&) = delete;

	public:

		//TODO 頂点バッファ取得の関数はユーザーには触れてほしくないので隠す
		dx12::VertexBuffer* GetPositionBuffer() const { return position_buffer_.get(); }
		dx12::VertexBuffer* GetNormalBuffer() const { return normal_buffer_.get(); }
		dx12::VertexBuffer* GetTangentBuffer() const { return tangent_buffer_.get(); }
		dx12::VertexBuffer* GetBinormalBuffer() const { return binormal_buffer_.get(); }
		dx12::VertexBuffer* GetTexcoordBuffer() const { return texcoord_buffer_.get(); }
		dx12::IndexBuffer*	GetIndexBuffer() const { return index_buffer_.get(); }

		/**
		 * \brief 描画用の生データ取得
		 * \return MeshData
		 */
		const Data&						GetRenderingData()				const { return data_; }

		/**
		 * \brief 描画用の頂点座標生データ
		 * \return Mesh Position Vec
		 */
		const std::vector<Position>&	GetRenderingPositionVec()		const { return data_.position_vec_; }

		/**
		 * \brief 描画用の頂点法線生データ
		 * \return Mesh Normal Vec
		 */
		const std::vector<Normal>&		GetRenderingNormalVec()			const { return data_.normal_vec_; }

		/**
		 * \brief 描画用の頂点接線生データ
		 * \return Mesh Tangent Vec
		 */
		const std::vector<Tangent>&		GetRenderingTangentVec()		const { return data_.tangent_vec_; }

		/**
		 * \brief 描画用の頂点従法線生データ
		 * \return Mesh Binormal Vec
		 */
		const std::vector<Binormal>&	GetRenderingBinormalVec()		const { return data_.binormal_vec_; }

		/**
		 * \brief 描画用の頂点UV座標生データ
		 * \return Mesh Texcoord Vec
		 */
		const std::vector<Texcoord>&	GetRenderingTexcoordVec()		const { return data_.texcoord_vec_; }

		/**
		 * \brief 描画に使われている頂点の数を取得
		 * \return 描画に使われている頂点数
		 */
		size_t							GetRenderingVertexCount()		const { return data_.position_vec_.size(); }

		/**
		 * \brief 描画に使われているインデックスの数を取得
		 * \return 描画に使われているインデックス
		 */
		size_t							GetRenderingIndexCount()		const { return data_.position_vec_.size(); }

		/**
		 * \brief 当たり判定のMesh生データを取得
		 * \return 当たり判定用のMesh生データ
		 */
		const Data&						GetCollisionData()				const { return collision_data_; }

		/**
		 * \brief 当たり判定用の頂点座標データを取得
		 * \return 当たり判定用の頂点座標データ
		 */
		const std::vector<Position>&	GetCollisionPositionVec()		const { return collision_data_.position_vec_; }

		/**
		 * \brief 当たり判定用の頂点法線データを取得
		 * \return 当たり判定用の頂点法線データ 
		 */
		const std::vector<Normal>&		GetCollisionNormalVec()			const { return collision_data_.normal_vec_; }

		/**
		 * \brief 当たり判定用の頂点接線データを取得
		 * \return 当たり判定用の頂点接線データ 
		 */
		const std::vector<Tangent>&		GetCollisionTangentVec()		const { return collision_data_.tangent_vec_; }

		/**
		 * \brief 当たり判定用の頂点従法線データを取得
		 * \return 当たり判定用の頂点従法線データ
		 */
		const std::vector<Binormal>&	GetCollisionBinormalVec()		const { return collision_data_.binormal_vec_; }

		/**
		 * \brief 当たり判定用の頂点UV座標データを取得
		 * \return 当たり判定用の頂点UV座標データ
		 */
		const std::vector<Texcoord>&	GetCollisionTexcoordVec()		const { return collision_data_.texcoord_vec_; }

		/**
		 * \brief メッシュのグローバル空間でのデフォルトトランスフォーム　
		 * これは描画用のデータと当たり判定用のデータで共通のものであるとする(描画用のデータで当たり判定用のデータを上書きするものとする)
		 * \return グローバル空間でのデフォルトトランスフォーム
		 */
		DirectX::XMFLOAT4X4				GetDefaultGlobalTransform()		const { return data_.default_global_transform_; }

		/**
		 * \brief 当たり判定用のデータをセット
		 * \param data 当たり判定用のデータをセット
		 */
		void							SetCollisionData(const Data& data)
		{
			collision_data_ = data; collision_data_.default_global_transform_ = data_.default_global_transform_;
		}

		//TODO 複数のマテリアルに対応した関数を作る
		/**
		 * \brief Materialを取得 !!!そのうち消します
		 * \return Materialへのポインタ
		 */
		std::shared_ptr<Material> GetMaterial() const { return material_; }

	private:

		/**
		 * \brief 描画API依存のバッファを作成する
		 */
		void Awake(const GraphicsContext* graphics_context);

		/**
		 * \brief Guiに描画
		 */
		void OnGui() override;
	private:

		//TODO メモリ使用量から考えると描画用の生頂点データを保持するのは良くないかもしれない
		//当たり判定用の生データを保持するべきかな…？？
		Data data_;		//生のデータ
		Data collision_data_;	//当たり判定用のデータ(レイキャスト用)	引数で入ってこなかった場合は描画用のデータと共通

		//バッファオブジェクト
		std::unique_ptr<dx12::VertexBuffer> position_buffer_{};
		std::unique_ptr<dx12::VertexBuffer> normal_buffer_{};
		std::unique_ptr<dx12::VertexBuffer> tangent_buffer_{};
		std::unique_ptr<dx12::VertexBuffer> binormal_buffer_{};
		std::unique_ptr<dx12::VertexBuffer> texcoord_buffer_{};
		std::unique_ptr<dx12::IndexBuffer> index_buffer_{};

		//In Progress レイトレーシング用の構造体
		uint64_t blas_unique_id_;

		//TODO 複数マテリアルへの対応 レイトレの場合はどうすればいいでしょうね
		//マテリアルへのポインタ or Unique ID
		std::shared_ptr<Material> material_;

	};
}