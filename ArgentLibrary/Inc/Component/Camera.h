#pragma once

#include <DirectXMath.h>

#include <memory>

#include "Component.h"
#include "Transform.h"

namespace argent::component
{
	class Camera final : public  Component
	{
	public:

		Camera();
		~Camera() override = default;

		Camera(const Camera&) = delete;
		Camera(const Camera&&) = delete;
		Camera& operator=(const Camera&) = delete;
		Camera& operator=(const Camera&&) = delete;

		void OnGui() override;

		/**
		 * \brief ビュー行列を計算する
		 * \return View Matrix
		 */
		DirectX::XMFLOAT4X4 CalcViewMatrix() const;

		/**
		 * \brief プロジェクション行列を計算する
		 * \return Projection Matrix
		 */
		DirectX::XMFLOAT4X4 CalcProjectionMatrix() const;

		/**
		 * \brief ビュープロジェクション行列を計算する
		 * \return View Projection Matrix
		 */
		DirectX::XMFLOAT4X4 CalcViewProjectionMatrix() const;

		/**
		 * \brief カメラのFocus座標をセット
		 * !!! auto focus update == falseになります。内部処理の事情です !!!
		 * 毎フレームあるいはカメラの座標更新が行われるたびに呼ばないとEye == Focusとなって
		 * 落ちる場合があるので注意
		 * \param focus Focus Position
		 */
		void SetLookAt(const DirectX::XMFLOAT3& focus) { focus_ = focus; auto_focus_update_ = false; }

		/**
		 * \brief アスペクト比をセット
		 * \param aspect_ratio Aspect Ratio
		 */
		void SetAspectRatio(float aspect_ratio) { aspect_ratio_ = aspect_ratio; }

		/**
		 * \brief Fov Angleをセット
		 * \param fov_y Fov Angle Y
		 */
		void SetFov(float fov_y) { fov_y_ = fov_y; }

		/**
		 * \brief Near Z Panelをセット
		 * \param near_z Near Z カメラEyeからの距離
		 */
		void SetNearZ(float near_z) { near_z_ = near_z; }

		/**
		 * \brief Far Z Panleをセット
		 * \param far_z Far Z カメラEyeからの距離
		 */
		void SetFarZ(float far_z) { far_z_ = far_z; }

		/**
		 * \brief 自動的なFocusの更新を許可するかどうか
		 * 許可した場合、Focusの座標はEyeの座標＋前方単位ベクトル
		 * \param allow_auto_focus_update Is allow automatic focus update
		 */
		void SetAllowAutoFocusUpdate(bool allow_auto_focus_update) { auto_focus_update_ = allow_auto_focus_update; }


		/**
		 * \brief カメラのFocus座標を取得
		 * !!! auto focus update == trueの場合は正常に機能しないので注意 !!!
		 * \return Look At Position
		 */
		DirectX::XMFLOAT3 GetLookAt() const { return focus_; }

		/**
		 * \brief アスペクト比を取得
		 * \return Aspect Ratio
		 */
		float GetAspectRatio() const { return aspect_ratio_; }

		/**
		 * \brief Fovアングルを取得(Radian)
		 * \return Fov Angle(Radian)
		 */
		float GetFov() const { return fov_y_; }

		/**
		 * \brief Far Panelを取得
		 * \return Far Panel
		 */
		float GetNearZ() const { return near_z_; }

		/**
		 * \brief Near Panelを取得
		 * \return Near Panel
		 */
		float GetFarZ() const { return far_z_; }

		/**
		 * \brief 自動的なFocusのUpdateを許可しているか取得
		 * 許可していた場合、Focus = Eye + Normalized Forward Vec
		 * \return Is allow automatic Focus Update
		 */
		bool GetAllowAutoFocusUpdate() const { return auto_focus_update_; }

		/**
		 * \brief カメラの座標を取得
		 * \return Eye Position
		 */
		DirectX::XMFLOAT3 GetEye() const { return transform_->GetLocalPosition(); }

		/**
		 * \brief Transformを取得
		 * \return Transform
		 */
		Transform* GetTransform() const { return transform_.get(); }

	private:

		std::unique_ptr<Transform> transform_{};
		DirectX::XMFLOAT3 focus_;
		float aspect_ratio_;
		float fov_y_;
		float near_z_;
		float far_z_;
		bool auto_focus_update_;
	};
}

