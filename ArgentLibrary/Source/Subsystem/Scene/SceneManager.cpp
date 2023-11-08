#include "Subsystem/Scene/SceneManager.h"

#include "Subsystem/Scene/BaseScene.h"


namespace argent::scene
{

	class DerivedScene : public BaseScene
	{
	public:
		DerivedScene() = default;
		~DerivedScene() override = default;

		void Awake() override
		{
			int i = 0;
			BaseScene::Awake();
		}
	};

	SceneManager::SceneManager():
		current_scene_(nullptr)
	{
		next_scene_name_.clear();
		scene_array_.clear();
		//Register("Derived Scene", new DerivedScene);
		//SetNextScene("Derived Scene");
	}

	void SceneManager::Awake()
	{
		Subsystem::Awake();
	}

	void SceneManager::Shutdown()
	{
		if(current_scene_)
			current_scene_->Shutdown();
	}

	void SceneManager::Update()
	{
		ChangeScene();

		if(current_scene_)
			current_scene_->Update();
	}

	void SceneManager::Render(const graphics::RenderContext* render_context) const
	{
		//if(current_scene_)
			//current_scene_->Render(render_context);
	}


	void SceneManager::OnGui() const
	{
		if(current_scene_)
			current_scene_->OnGui();
	}

	void SceneManager::Register(std::string scene_name, BaseScene* scene)
	{
		scene_array_[scene_name].reset(scene);
	}

	void SceneManager::ChangeScene()
	{
		if(next_scene_name_.empty()) return;

		//�w�肳��Ė��O�̃V�[�����擾
		auto next_scene = scene_array_.find(next_scene_name_);
		if(next_scene == scene_array_.end()) _ASSERT_EXPR(false, L"Next Scene Name�ɓo�^����Ă��Ȃ��V�[���̖��O�������Ă��܂�");

		//���݂̃V�[�����I��
		if(current_scene_)
			current_scene_->Shutdown();

		//�V�[���̐؂�ւ�&����������
		current_scene_ = (*next_scene).second.get();
		if(!current_scene_) _ASSERT_EXPR(false, L"Next Scene�Ɏw�肳�ꂽ�V�[����nullptr�ł���");

		current_scene_->Awake();
		int i = 0;

		//���O��Null��
		next_scene_name_.clear();
	}
}
