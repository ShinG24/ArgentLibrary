#pragma once

#include <vector>
#include <memory>

#include "GameResource.h"

namespace argent::graphics
{
	struct GraphicsContext;
}

namespace argent::graphics
{
	class Mesh;
	class Material;

	/**
	 * \brief ���f���N���X
	 * Mesh��Material�𕡐��ێ����Ă���A�I�v�V������Bone��Animation��ێ�����
	 * Mesh��Material�͈�ȏ�ێ����Ă��邱�Ƃ��ۏ؂���Ă���(�����Ă��Ȃ��ꍇ��Assertion���Ă΂��)
	 * �A�j���[�V��������ꍇ�̂�Bone��Animation��ێ�����
	 */
	class Model final : GameResource
	{
	public:
		Model() = default;
		~Model() override = default;

		/**
		 * \brief �R���X�g���N�^�@�f�[�^��std::move()�ňڍs�����邽�ߏ��L�����Ȃ��Ȃ邱�Ƃɒ���
		 * \param filepath Model�̃p�X(.exe����̑��΃p�X) 
		 * \param mesh_vec Mesh�̃f�[�^
		 * \param material_vec Material�̃f�[�^
		 */
		Model(std::string& filepath, std::vector<std::shared_ptr<Mesh>>& mesh_vec,
		      std::vector<std::shared_ptr<Material>>& material_vec);

		void Awake(const GraphicsContext* graphics_context);

		Model(const Model&) = delete;
		Model(const Model&&) = delete;
		Model& operator=(const Model&) = delete;
		Model& operator=(const Model&&) = delete;

		/**
		 * \brief Gui�ɕ`�悷��p�̊֐�
		 */
		void OnGui() override;

	private:
		std::string filepath_;	//�t�@�C���p�X
		std::vector<std::shared_ptr<Mesh>> mesh_vec_;
		std::vector<std::shared_ptr<Material>> material_vec_;
	};
}