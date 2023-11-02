#pragma once
#include <DirectXMath.h>

#include "Component.h"

namespace argent::component
{
	/**
	 * \brief �ړ��l�A�g�k�l�A��]�l���������R���|�[�l���g
	 * �R���|�[�l���g�w���̏ꍇ�A�S�Ă�GameObject�͂���Component�������Ă���
	 * Rotation��float4 quaternion�ɂ�����
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

		//Gui��ɕ`��
		void OnGui() override;

		/**
		 * \brief ���[���h�ϊ��s����v�Z���Ď擾
		 * �����ɉ�]�s�񂩂�forward, right, up�̒l���X�V
		 * \return World Matrix
		 */
		DirectX::XMFLOAT4X4 CalcWorldMatrix();

		/**
		 * \brief ���[�J����Ԃł̍��W�l���擾
		 * \return Position
		 */
		DirectX::XMFLOAT3 GetLocalPosition() const { return position_; }

		/**
		 * \brief ���[�J����Ԃł̊g�k�l���擾
		 * \return Scale
		 */
		DirectX::XMFLOAT3 GetLocalScale() const { return scale_; }

		/**
		 * \brief ���[�J����Ԃł̉�]�l���擾
		 * \return Rotation
		 */
		DirectX::XMFLOAT3 GetLocalRotation() const { return rotation_; }

		/**
		 * \brief ���[�J����Ԃł̍��W�l���Z�b�g
		 * �e�����Ȃ��ꍇ�A���[�J����ԁ������[���h���
		 * \param position Position
		 */
		void SetPosition(const DirectX::XMFLOAT3& position) { position_ = position; }

		/**
		 * \brief ���[�J����Ԃł̊g�k�l���Z�b�g
		 * �e�����Ȃ��ꍇ�A���[�J����ԁ����O���[�o�����
		 * \param scale Scale
		 */
		void SetScale(const DirectX::XMFLOAT3& scale) { scale_ = scale; }

		/**
		 * \brief ���[�J����Ԃł̉�]�l���Z�b�g
		 * �e�����Ȃ��ꍇ�A���[�J����ԁ����O���[�o�����
		 * \param rotation Rotation
		 */
		void SetRotation(const DirectX::XMFLOAT3& rotation) { rotation_ = rotation; }

		/**
		 * \brief �P�ʑO���x�N�g�����擾
		 * ���̃x�N�g���͓r����CalcForward()���Ăяo���Ă���ꍇ������,
		 * 1�t���[���O�̕`��Ŏg�p���ꂽ����
		 * \return Forward Vector
		 */
		DirectX::XMFLOAT3 GetForward() const { return forward_; }

		/**
		 * \brief �P�ʏ���x�N�g�����擾
		 * ���̃x�N�g���͓r����CalcUp()���Ăяo���Ă���ꍇ�������A
		 * 1�t���[���O�̕`��Ŏg�p���ꂽ����
		 * \return Up Vector
		 */
		DirectX::XMFLOAT3 GetUp() const { return up_; }

		/**
		 * \brief �P�ʉE���x�N�g�����擾
		 * ���̃x�N�g���͓r����CalcRight()���Ăяo���Ă���ꍇ�������A
		 * 1�t���[���O�̕`��Ŏg�p���ꂽ����
		 * \return Right Vector
		 */
		DirectX::XMFLOAT3 GetRight() const { return right_; }

		/**
		 * \brief �����x�N�g�����v�Z
		 * �O���A�E���A����A���ׂĂ̒P�ʕ����x�N�g�����v�Z�A�X�V����
		 */
		void CalcDirectionalVector();

		/**
		 * \brief �P�ʑO���x�N�g�����v�Z���Ă���擾
		 * �v�Z�R�X�g����l���āA���̊֐���CalcUp(), CalcRight()�𕡐���Ăԉ\��������ꍇ�A
		 * ���̓s�x���m�Ȓl���K�v�ȏꍇ�������ACalcDirectionalVector()�őS�Ă̕����x�N�g�����X�V���A
		 * Get~()�Ŏ擾����̂��]�܂���
		 * \return Forward Vector
		 */
		DirectX::XMFLOAT3 CalcForward();

		/**
		 * \brief �P�ʏ���x�N�g�����v�Z���Ă���擾
		 * �v�Z�R�X�g����l���āA���̊֐���CalcForward(), CalcRight()�𕡐���Ăԉ\��������ꍇ�A
		 * ���̓s�x���m�Ȓl���K�v�ȏꍇ�������ACalcDirectionalVector()�őS�Ă̕����x�N�g�����X�V���A
		 * Get~()�Ŏ擾����̂��]�܂���
		 * \return Up Vector 
		 */
		DirectX::XMFLOAT3 CalcUp();

		/**
		 * \brief �P�ʉE���x�N�g�����v�Z���Ă���擾
		 * �v�Z�R�X�g����l���āA���̊֐���CalcForward(), CalcRight()�𕡐���Ăԉ\��������ꍇ�A
		 * ���̓s�x���m�Ȓl���K�v�ȏꍇ�������ACalcDirectionalVector()�őS�Ă̕����x�N�g�����X�V���A
		 * Get~()�Ŏ擾����̂��]�܂���
		 * \return Right Vector
		 */
		DirectX::XMFLOAT3 CalcRight();

		/**
		 * \brief ���W�𑫂�
		 * \param pos Pos
		 */
		void AddPosition(const DirectX::XMFLOAT3& pos);

		/**
		 * \brief ��]�l�𑫂�
		 * \param rot Rot
		 */
		void AddRotation(const DirectX::XMFLOAT3& rot);
	private:

		DirectX::XMFLOAT3 position_;	//���W
		DirectX::XMFLOAT3 scale_;		//�g�k
		DirectX::XMFLOAT3 rotation_;	//��]
		DirectX::XMFLOAT3 forward_;		//�O���x�N�g��
		DirectX::XMFLOAT3 right_;		//�E���x�N�g��
		DirectX::XMFLOAT3 up_;			//����x�N�g��
	};
}

