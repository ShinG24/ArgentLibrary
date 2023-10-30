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

		/**
		 * \brief �r���[�s����v�Z����
		 * \return View Matrix
		 */
		DirectX::XMFLOAT4X4 CalcViewMatrix() const;

		/**
		 * \brief �v���W�F�N�V�����s����v�Z����
		 * \return Projection Matrix
		 */
		DirectX::XMFLOAT4X4 CalcProjectionMatrix() const;

		/**
		 * \brief �r���[�v���W�F�N�V�����s����v�Z����
		 * \return View Projection Matrix
		 */
		DirectX::XMFLOAT4X4 CalcViewProjectionMatrix() const;

		/**
		 * \brief �J������Focus���W���Z�b�g
		 * !!! auto focus update == false�ɂȂ�܂��B���������̎���ł� !!!
		 * ���t���[�����邢�̓J�����̍��W�X�V���s���邽�тɌĂ΂Ȃ���Eye == Focus�ƂȂ���
		 * ������ꍇ������̂Œ���
		 * \param focus Focus Position
		 */
		void SetLookAt(const DirectX::XMFLOAT3& focus) { focus_ = focus; auto_focus_update_ = false; }

		/**
		 * \brief �A�X�y�N�g����Z�b�g
		 * \param aspect_ratio Aspect Ratio
		 */
		void SetAspectRatio(float aspect_ratio) { aspect_ratio_ = aspect_ratio; }

		/**
		 * \brief Fov Angle���Z�b�g
		 * \param fov_y Fov Angle Y
		 */
		void SetFov(float fov_y) { fov_y_ = fov_y; }

		/**
		 * \brief Near Z Panel���Z�b�g
		 * \param near_z Near Z �J����Eye����̋���
		 */
		void SetNearZ(float near_z) { near_z_ = near_z; }

		/**
		 * \brief Far Z Panle���Z�b�g
		 * \param far_z Far Z �J����Eye����̋���
		 */
		void SetFarZ(float far_z) { far_z_ = far_z; }

		/**
		 * \brief �����I��Focus�̍X�V�������邩�ǂ���
		 * �������ꍇ�AFocus�̍��W��Eye�̍��W�{�O���P�ʃx�N�g��
		 * \param allow_auto_focus_update Is allow automatic focus update
		 */
		void SetAllowAutoFocusUpdate(bool allow_auto_focus_update) { auto_focus_update_ = allow_auto_focus_update; }


		/**
		 * \brief �J������Focus���W���擾
		 * !!! auto focus update == true�̏ꍇ�͐���ɋ@�\���Ȃ��̂Œ��� !!!
		 * \return Look At Position
		 */
		DirectX::XMFLOAT3 GetLookAt() const { return focus_; }

		/**
		 * \brief �A�X�y�N�g����擾
		 * \return Aspect Ratio
		 */
		float GetAspectRatio() const { return aspect_ratio_; }

		/**
		 * \brief Fov�A���O�����擾(Radian)
		 * \return Fov Angle(Radian)
		 */
		float GetFov() const { return fov_y_; }

		/**
		 * \brief Far Panel���擾
		 * \return Far Panel
		 */
		float GetNearZ() const { return near_z_; }

		/**
		 * \brief Near Panel���擾
		 * \return Near Panel
		 */
		float GetFarZ() const { return far_z_; }

		/**
		 * \brief �����I��Focus��Update�������Ă��邩�擾
		 * �����Ă����ꍇ�AFocus = Eye + Normalized Forward Vec
		 * \return Is allow automatic Focus Update
		 */
		bool GetAllowAutoFocusUpdate() const { return auto_focus_update_; }

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

