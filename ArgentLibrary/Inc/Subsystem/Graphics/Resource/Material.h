#pragma once
#include "GameResource.h"

#include <memory>
#include <unordered_map>

#include "Texture.h"

namespace argent::graphics::dx12
{
	struct Descriptor;
	class ConstantBuffer;
}

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
						//Diffuse�e�N�X�`���͂���ɊY������O��őg��ł܂�
			Normal,		//Normal Map �ڐ���Ԃ̖@�������i�[�����e�N�X�`���@�@���̌v�Z�Ɏg��
			Metallic,	//Metallic	��������\�����߂̃e�N�X�`���@�f�t�H���g�͑������̃e�N�X�`��
			Roughness,	//Roughness �\�ʂ̑e����\�����邽�߂̃e�N�X�`�� �f�t�H���g�͑������̃e�N�X�`��
			Emissive,	//Emissive	�P�x���ߗp�H�̃e�N�X�`���@�P���������܂�
			Height,		//Height Map �����Ƃ��Ɏg���炵���@�����͏o���ĂȂ�
			Mask,		//Mask		�����F�X�g���邯�ǒm��Ȃ�
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
		 * \brief �e�N�X�`���̃A�b�v���[�h���I������܂ő҂�
		 */
		virtual void WaitForGpu();

		/**
		 * \brief�@�R���X�^���g�o�b�t�@��Update
		 * \param frame_index �t���[���C���f�b�N�X
		 */
		virtual void UpdateConstantBuffer(UINT frame_index) = 0;

		/**
		 * \brief �}�e���A���C���X�^���X�ł̃e�N�X�`���̎g��������Y���̃e�N�X�`�����擾����
		 * ���������Ă��Ȃ������ꍇ��Nullptr���ς���
		 * \param type �C���X�^���X�ł̃e�N�X�`���̎g�p�p�r
		 * \return Texture�ւ̃|�C���^
		 */
		std::shared_ptr<Texture> GetTexture(TextureUsage type);

		/**
		 * \brief �e�N�X�`����GPU�n���h�����擾����
		 * //TODO ���̊֐��̓}�e���A�����ێ����Ă���e�N�X�`���������f�B�X�N���v�^�q�[�v��ɑ��݂��A
		 * ���A�����Ă���O��őg��ł���̂ŕύX����
		 * \return d3d12 Gpu Descriptor handle
		 */
		D3D12_GPU_DESCRIPTOR_HANDLE GetTextureGpuHandleBegin() const { return texture_map_.begin()->second->GetGpuHandle(); }

		/**
		 * \brief �R���X�^���g�o�b�t�@��Gpu�A�h���X���擾
		 * \param frame_index �t���[���C���f�b�N�X
		 * \return gpu address
		 */
		D3D12_GPU_VIRTUAL_ADDRESS GetConstantGpuVirtualAddress(UINT frame_index) const;

		/**
		 * \brief �R���X�^���g�o�b�t�@��DescriptorHandle���擾
		 * \param frame_index �t���[���C���f�b�N�X
		 * \return descriptor
		 */
		dx12::Descriptor GetConstantDescriptorHandle(UINT frame_index) const;

		//�p���O��̑g�ݕ��Ōp���悩��A�N�Z�X����\�����傢�ɂ��邽�߁A�������x���l��Getter�ł͂Ȃ�protected�ɂ��ăA�N�Z�X�ł���悤�ɂ���
	protected:
		//TODO �e�N�X�`���������Ȃ��}�e���A���͑��݂���̂� �������Ƃ��Ă��e�N�X�`���}�b�v���g��Ȃ���΂��������Ȃ̂�
		//�Ƃ肠���������Ă����܂�
		std::unordered_map<TextureUsage, std::shared_ptr<Texture>> texture_map_;	//Texture map�@2�ȏ�̎g�p�p�r��������e�N�X�`���͕ێ��ł��Ȃ� 
		std::unique_ptr<dx12::ConstantBuffer> constant_buffer_;
	};
}
