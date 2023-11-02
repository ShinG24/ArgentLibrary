#pragma once

#include <DirectXMath.h>

#include <memory>

#include "Subsystem/Subsystem.h"


namespace argent::graphics::dx12
{
	class ConstantBuffer;
}

namespace argent::graphics
{
	struct GraphicsContext;
	struct RenderContext;

	class Raytracer;

	/**
	 * \brief �`��}�l�[�W���@�`��Ǘ����s��
	 * ���C�g���[�V���O��p���邩�ǂ����������Ő؂�ւ���
	 */
	class RenderingManager final : public Subsystem
	{
	public:

		struct SceneConstant
		{
			DirectX::XMFLOAT4 camera_position_;
			DirectX::XMFLOAT4X4 view_matrix_;
			DirectX::XMFLOAT4X4 projection_matrix_;
			DirectX::XMFLOAT4X4 view_projection_matrix_;
			DirectX::XMFLOAT4X4 inv_view_projection_matrix_;
			DirectX::XMFLOAT4 light_direction_;
		};

	public:

		RenderingManager() = default;
		~RenderingManager() override = default;

		RenderingManager(const RenderingManager&) = delete;
		RenderingManager(const RenderingManager&&) = delete;
		RenderingManager& operator=(const RenderingManager&) = delete;
		RenderingManager& operator=(const RenderingManager&&) = delete;

		/**
		 * \brief ������
		 * �V�[���R���X�^���g�o�b�t�@�̍쐬
		 */
		void Awake() override;

		/**
		 * \brief �I������
		 */
		void Shutdown() override;

		void OnGui();

		/**
		 * \brief �J�n���߁@
		 * �`��ɕK�v�ȃf�[�^�i�J�����A���C�g�Ȃǁj�̏����W�߂�B
		 */
		void FrameBegin(const RenderContext* render_context, const SceneConstant& scene_data);

		/**
		 * \brief �`��I��
		 */
		void FrameEnd() const;

		/**
		 * \brief ���C�g���[�V���O�̎��s
		 * \param render_context RenderContext
		 * \param graphics_context Graphics Context
		 */
		void OnRaytrace(const RenderContext* render_context, const GraphicsContext* graphics_context) const;

		/**
		 * \brief ���C�g�����[�h���I�����ǂ���
		 * \return Is On Raytracing Mode
		 */
		bool IsRaytracing() const { return on_raytrace_; }

	private:

		std::unique_ptr<dx12::ConstantBuffer> scene_constant_buffer_;
		SceneConstant scene_data_{};
		bool on_raytrace_{ true };

		//TODO ������܂Ƃ��Ȍ`�ɂ���
		std::unique_ptr<Raytracer> raytracer_;
	};
}

