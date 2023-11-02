#pragma once
#include <DirectXMath.h>

#include "Component.h"

namespace argent::component
{
	/**
	 * \brief 移動値、拡縮値、回転値を持ったコンポーネント
	 * コンポーネント指向の場合、全てのGameObjectはこのComponentを持っている
	 * Rotationをfloat4 quaternionにしたい
	 */
	class Transform final : public Component
	{
	public:

		Transform();
		~Transform() override = default;

		Transform(const Transform&) = delete;
		Transform(const Transform&&) = delete;
		Transform& operator=(const Transform&) = delete;
		Transform& operator=(const Transform&&) = delete;

		//Gui上に描画
		void OnGui() override;

		/**
		 * \brief ワールド変換行列を計算して取得
		 * 同時に回転行列からforward, right, upの値を更新
		 * \return World Matrix
		 */
		DirectX::XMFLOAT4X4 CalcWorldMatrix();

		/**
		 * \brief ローカル空間での座標値を取得
		 * \return Position
		 */
		DirectX::XMFLOAT3 GetLocalPosition() const { return position_; }

		/**
		 * \brief ローカル空間での拡縮値を取得
		 * \return Scale
		 */
		DirectX::XMFLOAT3 GetLocalScale() const { return scale_; }

		/**
		 * \brief ローカル空間での回転値を取得
		 * \return Rotation
		 */
		DirectX::XMFLOAT3 GetLocalRotation() const { return rotation_; }

		/**
		 * \brief ローカル空間での座標値をセット
		 * 親がいない場合、ローカル空間＝＝ワールド空間
		 * \param position Position
		 */
		void SetPosition(const DirectX::XMFLOAT3& position) { position_ = position; }

		/**
		 * \brief ローカル空間での拡縮値をセット
		 * 親がいない場合、ローカル空間＝＝グローバル空間
		 * \param scale Scale
		 */
		void SetScale(const DirectX::XMFLOAT3& scale) { scale_ = scale; }

		/**
		 * \brief ローカル空間での回転値をセット
		 * 親がいない場合、ローカル空間＝＝グローバル空間
		 * \param rotation Rotation
		 */
		void SetRotation(const DirectX::XMFLOAT3& rotation) { rotation_ = rotation; }

		/**
		 * \brief 単位前方ベクトルを取得
		 * このベクトルは途中でCalcForward()を呼び出している場合を除き,
		 * 1フレーム前の描画で使用されたもの
		 * \return Forward Vector
		 */
		DirectX::XMFLOAT3 GetForward() const { return forward_; }

		/**
		 * \brief 単位上方ベクトルを取得
		 * このベクトルは途中でCalcUp()を呼び出している場合を除き、
		 * 1フレーム前の描画で使用されたもの
		 * \return Up Vector
		 */
		DirectX::XMFLOAT3 GetUp() const { return up_; }

		/**
		 * \brief 単位右方ベクトルを取得
		 * このベクトルは途中でCalcRight()を呼び出している場合を除き、
		 * 1フレーム前の描画で使用されたもの
		 * \return Right Vector
		 */
		DirectX::XMFLOAT3 GetRight() const { return right_; }

		/**
		 * \brief 方向ベクトルを計算
		 * 前方、右方、上方、すべての単位方向ベクトルを計算、更新する
		 */
		void CalcDirectionalVector();

		/**
		 * \brief 単位前方ベクトルを計算してから取得
		 * 計算コストから考えて、この関数やCalcUp(), CalcRight()を複数回呼ぶ可能性がある場合、
		 * その都度正確な値が必要な場合を除き、CalcDirectionalVector()で全ての方向ベクトルを更新し、
		 * Get~()で取得するのが望ましい
		 * \return Forward Vector
		 */
		DirectX::XMFLOAT3 CalcForward();

		/**
		 * \brief 単位上方ベクトルを計算してから取得
		 * 計算コストから考えて、この関数やCalcForward(), CalcRight()を複数回呼ぶ可能性がある場合、
		 * その都度正確な値が必要な場合を除き、CalcDirectionalVector()で全ての方向ベクトルを更新し、
		 * Get~()で取得するのが望ましい
		 * \return Up Vector 
		 */
		DirectX::XMFLOAT3 CalcUp();

		/**
		 * \brief 単位右方ベクトルを計算してから取得
		 * 計算コストから考えて、この関数やCalcForward(), CalcRight()を複数回呼ぶ可能性がある場合、
		 * その都度正確な値が必要な場合を除き、CalcDirectionalVector()で全ての方向ベクトルを更新し、
		 * Get~()で取得するのが望ましい
		 * \return Right Vector
		 */
		DirectX::XMFLOAT3 CalcRight();

		/**
		 * \brief 座標を足す
		 * \param pos Pos
		 */
		void AddPosition(const DirectX::XMFLOAT3& pos);

		/**
		 * \brief 回転値を足す
		 * \param rot Rot
		 */
		void AddRotation(const DirectX::XMFLOAT3& rot);
	private:

		DirectX::XMFLOAT3 position_;	//座標
		DirectX::XMFLOAT3 scale_;		//拡縮
		DirectX::XMFLOAT3 rotation_;	//回転
		DirectX::XMFLOAT3 forward_;		//前方ベクトル
		DirectX::XMFLOAT3 right_;		//右方ベクトル
		DirectX::XMFLOAT3 up_;			//上方ベクトル
	};
}

