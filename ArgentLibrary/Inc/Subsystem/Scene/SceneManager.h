#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Subsystem/Subsystem.h"

namespace argent::graphics
{
	struct RenderContext;
}

namespace argent::scene
{
	class BaseScene;

	class SceneManager final : public Subsystem
	{
	public:

		SceneManager();
		~SceneManager() override = default;

		SceneManager(const SceneManager&) = delete;
		SceneManager(const SceneManager&&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;
		SceneManager& operator=(const SceneManager&&) = delete;

		/**
		 * \brief ����������
		 */
		void Awake() override;

		/**
		 * \brief �I������
		 */
		void Shutdown() override;

		/**
		 * \brief �X�V����
		 * �V�[���̑J�ڂ��s��
		 * Current Scene��Nullptr�ł͂Ȃ��Ƃ��A
		 * Current Scene��Update���Ă΂��
		 */
		void Update();

		/**
		 * \brief �`��֐�
		 */
		void Render(const graphics::RenderContext* render_context) const;

		/**
		 * \brief Gui��ɕ`�悷��֐�
		 */
		void OnGui() const;

		/**
		 * \brief �V�[���̓o�^
		 * \param scene_name �V�[���̖��O
		 * \param scene �V�[���ւ̃|�C���^ ���L�����ڂ邱�Ƃɒ���
		 */
		void Register(std::string scene_name, BaseScene* scene);

		/**
		 * \brief ���̃V�[�����Z�b�g����
		 * \param scene_name Next Scene Name
		 */
		void SetNextScene(const std::string& scene_name) { next_scene_name_ = scene_name; }

		/**
		 * \brief ���݂̃V�[�����擾
		 * \return Current Scene
		 */
		BaseScene* GetCurrentScene() const { return current_scene_; }

	private:

		void ChangeScene();

	private:

		BaseScene* current_scene_{};
		std::string next_scene_name_{};

		std::unordered_map<std::string, std::unique_ptr<BaseScene>> scene_array_;

	};
}

