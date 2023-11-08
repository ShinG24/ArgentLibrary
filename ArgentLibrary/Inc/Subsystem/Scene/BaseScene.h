#pragma once

#include <DirectXMath.h>

#include <string>

namespace argent::graphics
{
	struct RenderContext;	
}

namespace argent::scene
{
	/**
	 * \brief �V�[�����N���X
	 */
	class BaseScene
	{
	public:

		BaseScene() = default;
		virtual ~BaseScene() = default;

		BaseScene(const BaseScene&) = delete;
		BaseScene(const BaseScene&&) = delete;
		BaseScene& operator=(const BaseScene&) = delete;
		BaseScene& operator=(const BaseScene&&) = delete;

		/**
		 * \brief �������֐�
		 * ���̃V�[����Current Scene�Ƃ��Đݒ肳��čŏ��ɌĂ΂��
		 * ���̊֐��͏�L�̏����𖞂������тɌĂ΂��
		 */
		virtual void Awake()
		{
			int i = 0;
		}

		/**
		 * \brief �I���֐�
		 * ���̃V�[����Curren Scene�ł͂Ȃ��Ȃ����Ƃ��ɍŌ�ɌĂ΂��
		 * ���̊֐��͏�L�̏����𖞂������тɌĂ΂��
		 */
		virtual void Shutdown() {}

		/**
		 * \brief �X�V�֐�
		 * ���̃V�[����Current Scene�ł������A���t���[�����Ă΂��
		 * ���̊֐��͕K��Render�����O�ŌĂ΂��
		 */
		virtual void Update() {}


		/**
		 * \brief 3D��Ԃɕ`��
		 * \param render_context RenderContext 
		 */
		virtual void OnRender3D(const graphics::RenderContext* render_context) {}

		/**
		 * \brief 2D�̕`��@�|�X�g�v���Z�X���������Ȃ���p UI�Ƃ�
		 * \param render_context RenderContext
		 */
		virtual void OnRender2D(const graphics::RenderContext* render_context) {}

		/**
		 * \brief Gui��ɕ`�悷��֐�
		 */
		virtual void OnGui() {}

		std::string GetName() const { return name_; }

		//TODO ���̎����@���[�U�[���G��K�v�̂Ȃ��`�ɕς���
		virtual DirectX::XMFLOAT3 GetCameraPosition() const = 0;
		virtual DirectX::XMFLOAT4X4 GetViewMatrix() const = 0;
		virtual DirectX::XMFLOAT4X4 GetProjectionMatrix() const = 0;
		virtual DirectX::XMFLOAT3 GetLightDirection() const = 0;

	protected:

		std::string name_ ;
	};
}

