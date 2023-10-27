#pragma once

#include "Material.h"

namespace argent::graphics
{
	/**
	 * \brief �f�t�H���g��3DMesh�}�e���A�� PBR�}�e���A��
	 * FbxLoader���烂�f�����C���|�[�g�����ꍇ�A�����̃}�e���A���͂���
	 * ���^���b�N�A���t�l�X���x�������o�ϐ��Ƃ��Ď���
	 * �g�p�\�ȃe�N�X�`���^�C�v��Albedo, Normal, Metllic, Roughness��4��
	 * //TODO Height, Mask�ɂ��Ή�����
	 */
	class StandardMaterial final : Material
	{
	public:

		//�C���X�^���X�쐬�p�̃f�[�^
		struct Data
		{
			std::string name_;
			std::unordered_map<TextureUsage, std::string> filepath_map_;
			float metallic_factor_;
			float smoothness_factor_;
		};

		//�R���X�^���g�o�b�t�@�p�̃f�[�^
		struct ConstantData
		{
			//TODO Color���Ă������ق��������H�H
			float metallic_factor_ = 0.0f;
			float smoothness_factor_ = 0.5f;
		};

	public:

		StandardMaterial() = default;
		StandardMaterial(const Data& data);

		~StandardMaterial() override = default;

		void Awake(const GraphicsContext* graphics_context) override;
		void OnGui() override;
	private:
		Data data_;	//TODO �����͕ێ����Ă����K�v������̂��H
		ConstantData constant_data_;
	};

}