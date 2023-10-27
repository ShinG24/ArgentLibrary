#pragma once
#include "GameResource.h"

#include <memory>
#include <unordered_map>

namespace argent::graphics
{
	struct GraphicsContext;
	class Texture;
}

namespace argent::graphics
{
	class GameResource;

	/**
	 * \brief �}�e���A�����N���X
	 * ������������΂������킩��Ȃ��̂łقډ����������Ă��Ȃ�
	 */
	class Material : public GameResource
	{
	public:

		enum class TextureUsage
		{
			Albedo,		//Albedo (Base Color)	1�Ԋ�{�ȐF�̏o�͂�S���@�f�t�H���g�͐^�����ȃe�N�X�`��
			Normal,		//Normal Map �ڐ���Ԃ̖@�������i�[�����e�N�X�`���@�@���̌v�Z�Ɏg��
			Metallic,	//Metallic	��������\�����߂̃e�N�X�`���@�f�t�H���g�͑������̃e�N�X�`��
			Roughness,	//Roughness �\�ʂ̑e����\�����邽�߂̃e�N�X�`�� �f�t�H���g�͑������̃e�N�X�`��
		};

	public:

		Material() = default;

		/**
		 * \brief �R���X�g���N�^
		 * \param name Material name
		 */
		Material(const std::string& name);
		~Material() override = default;

		Material(const Material&) = delete;
		Material(const Material&&) = delete;
		Material& operator=(const Material&) = delete;
		Material& operator=(const Material&&) = delete;

	public:

		/**
		 * \brief �e�N�X�`�����邢�̓R���X�^���g�o�b�t�@�̏������֐�
		 * �}�e���A���͉\�Ȍ��蔖���\���ɂ��邽�߁A�����ł͉������Ȃ��悤�ɂ���
		 * \param graphics_context GraphicsContext
		 */
		virtual void Awake(const GraphicsContext* graphics_context) = 0;

		/**
		 * \brief �}�e���A���C���X�^���X�ł̃e�N�X�`���̎g��������Y���̃e�N�X�`�����擾����
		 * ���������Ă��Ȃ������ꍇ��Nullptr���ς���
		 * \param type �C���X�^���X�ł̃e�N�X�`���̎g�p�p�r
		 * \return Texture�ւ̃|�C���^
		 */
		std::shared_ptr<Texture> GetTexture(TextureUsage type);

		//�p���O��̑g�ݕ��Ōp���悩��A�N�Z�X����\�����傢�ɂ��邽�߁A�������x���l��Getter�ł͂Ȃ�portected�ɂ��ăA�N�Z�X�ł���悤�ɂ���
	protected:
		//TODO �e�N�X�`���������Ȃ��}�e���A���͑��݂���̂� �������Ƃ��Ă��e�N�X�`���}�b�v���g��Ȃ���΂��������Ȃ̂�
		//�Ƃ肠���������Ă����܂�
		std::unordered_map<TextureUsage, std::shared_ptr<Texture>> texture_map_;	//Texture map�@2�ȏ�̎g�p�p�r��������e�N�X�`���͕ێ��ł��Ȃ� 
	};
}
