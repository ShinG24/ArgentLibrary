#pragma once

#include <string>

namespace argent::graphics
{
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
		explicit GameResource(std::string name):
			name_(std::move(name)) {}

		virtual ~GameResource() = default;

		GameResource(const GameResource&) = delete;
		GameResource(const GameResource&&) = delete;
		GameResource& operator= (const GameResource&) = delete;
		GameResource& operator= (const GameResource&&) = delete;

		const std::string& GetName() const { return name_; }

		/**
		 * \brief ImGui�ɕ\������֐�
		 */
		virtual void OnGui() {}

	private:
		std::string name_;	
	};
}