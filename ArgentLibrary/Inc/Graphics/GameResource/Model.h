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

		/**
		 * \brief �R���X�g���N�^�@�f�[�^��std::move()�ňڍs�����邽�ߏ��L�����Ȃ��Ȃ邱�Ƃɒ���
		 * \param filepath Model�̃p�X(.exe����̑��΃p�X) 
		 * \param mesh_vec Mesh�̃f�[�^
		 * \param material_vec Material�̃f�[�^
		 */
		Model(std::string filepath, std::vector<std::shared_ptr<Mesh>>& mesh_vec,
		      std::vector<std::shared_ptr<Material>>& material_vec);

		~Model() override = default;

		Model(const Model&) = delete;
		Model(const Model&&) = delete;
		Model& operator=(const Model&) = delete;
		Model& operator=(const Model&&) = delete;

	public:

		//TODO�@���̉�3�̊֐��̓��[�U�[�ɂ͐G��Ăق����Ȃ��̂�private�ɂ���
		/**
		 * \brief �`��API�ˑ���Buffer�I�u�W�F�N�g���쐬 Const�ɂ��Ă邯�Ǖs����������ꍇ�͏����Ă�����
		 * \param graphics_context GraphcisContext�̃|�C���^
		 */
		void Awake(const GraphicsContext* graphics_context);

		/**
		 * \brief Gui�ɕ`�悷��p�̊֐�
		 */
		void OnGui() override;

		/**
		 * \brief �e�N�X�`���Ȃǂ�GPU�ւ̃A�b�v���[�h���I������܂ő҂�
		 */
		void WaitForUploadGpuResource();

		/**
		 * \brief .exe����̑��΃t�@�C���p�X�擾 
		 * \return �t�@�C���p�X
		 */
		const std::string& GetFilePath() const { return filepath_; }

		/**
		 * \brief ���f�����ێ����Ă���Mesh�����ׂĎ擾����֐�
		 * \return Mesh
		 */
		const std::vector<std::shared_ptr<Mesh>>& GetMeshes() const { return mesh_vec_; }

		/**
		 * \brief ���f�����ێ����Ă���Material�����ׂĎ擾����֐�
		 * \return Material
		 */
		const std::vector<std::shared_ptr<Material>>& GetMaterials() const { return material_vec_; }

	private:
		std::string filepath_;	//�t�@�C���p�X
		std::vector<std::shared_ptr<Mesh>> mesh_vec_;
		std::vector<std::shared_ptr<Material>> material_vec_;

		bool is_awake_ = false;
		//TODO �A�j���[�V�����ɑΉ�
	};
}