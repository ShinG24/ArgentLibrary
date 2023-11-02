#pragma once

#include <string>

namespace argent::graphics
{
	//TODO ���\�[�X�̃^�C�v�����@�Ƃ肠����enum����������@�K�v�Ȃ���A����Ȃ��Ȃ�̂Ă�
	enum class ResourceState
	{
		Animation,
		Material,
		Mesh,
		Model,
		Texture,
		Sprite,
	};

	//���ׂẴQ�[���Ŏg�����\�[�X�̊��N���X
	//�}�e���A���A���b�V���A�e�N�X�`���A�A�j���[�V�����ȂǂȂ�
	//���[�U�[�̖ڂɐG��郊�\�[�X�̂��Ƃ�GameResource�ƌĂ�
	class GameResource
	{
	public:

		GameResource() = default;
		//TODO ���\�[�X�}�l�[�W������胆�j�[�NID���������邱��
		explicit GameResource(std::string name):
			name_(std::move(name)) {}

		virtual ~GameResource() = default;

		GameResource(const GameResource&) = delete;
		GameResource(const GameResource&&) = delete;
		GameResource& operator= (const GameResource&) = delete;
		GameResource& operator= (const GameResource&&) = delete;


		/**
		 * \brief ImGui�ɕ\������֐�
		 */
		virtual void OnGui() {}

		const std::string& GetName() const { return name_; }

		//In Progress
		/**
		 * \brief Unique ID�擾�@id�̓��\�[�X�}�l�[�W�����甭�s����A�B��ł��邱�Ƃ��ۏ؂���Ă���
		 * \return Unique ID
		 */
		uint64_t GetUniqueId() const { return unique_id_; }

	private:

		std::string name_;
		uint64_t unique_id_{};

	};
}