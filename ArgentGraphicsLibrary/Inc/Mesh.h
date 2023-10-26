#pragma once

#include <DirectXMath.h>

#include <memory>
#include <vector>

#include "GameResource.h"

namespace argent::graphics::dx12
{
	
	class VertexBuffer;
	class IndexBuffer;
	class BottomLevelAccelerationStructure;
}

namespace argent::graphics
{
	struct GraphicsContext;

	using Position	= DirectX::XMFLOAT3;
	using Normal	= DirectX::XMFLOAT3;
	using Tangent	= DirectX::XMFLOAT3;
	using Binormal	= DirectX::XMFLOAT3;
	using Texcoord	= DirectX::XMFLOAT2;

	/**
	 * \brief 3DMeshのデータ保持クラス
	 * 生のデータを保持する他、API依存の頂点バファをインデックスバッファを持つ
	 */
	class Mesh : public GameResource
	{
	public:
		friend class Model;
		struct Data
		{
			std::vector<Position> position_vec_{};
			std::vector<Normal> normal_vec_{};
			std::vector<Tangent> tangent_vec_{};
			std::vector<Binormal> binormal_vec_{};
			std::vector<Texcoord> texcoord_vec_{};
			std::vector<uint32_t> index_vec_{};

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
		{}

		const Data&						GetData()			const { return data_; }
		const std::vector<Position>&	GetPositionVec()	const { return data_.position_vec_; }
		const std::vector<Normal>&		GetNormalVec()		const { return data_.normal_vec_; }
		const std::vector<Tangent>&		GetTangentVec()		const { return data_.tangent_vec_; }
		const std::vector<Binormal>&	GetBinormal()		const { return data_.binormal_vec_; }
		const std::vector<Texcoord>&	GetTexcoord()		const { return data_.texcoord_vec_; }
		size_t							GetVertexCount()	const { return data_.position_vec_.size(); }
		size_t							GetIndexCount()		const { return data_.position_vec_.size(); }
	private:

		/**
		 * \brief 描画API依存のバッファを作成する
		 */
		void Awake(const GraphicsContext* graphics_context);

	private:
		Data data_;		//生のデータ

		std::unique_ptr<dx12::VertexBuffer> position_buffer_{};
		std::unique_ptr<dx12::VertexBuffer> normal_buffer_{};
		std::unique_ptr<dx12::VertexBuffer> tangent_buffer_{};
		std::unique_ptr<dx12::VertexBuffer> binormal_buffer_{};
		std::unique_ptr<dx12::VertexBuffer> texcoord_buffer_{};
		std::unique_ptr<dx12::IndexBuffer> index_buffer_{};

		//In Progress レイトレーシング用の構造体
		std::unique_ptr<dx12::BottomLevelAccelerationStructure> blas_{};
	};
}