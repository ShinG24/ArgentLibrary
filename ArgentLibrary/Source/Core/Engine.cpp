#include "Core/Engine.h"

#include "Core/SubsystemLocator.h"

namespace argent
{
	static Engine* engine_instance = nullptr;

	Engine::Engine()
	{
		if(engine_instance) _ASSERT_EXPR(false, L"Engine�̃C���X�^���X�͂��łɐ�������Ă��܂�\n"
										  "�C���X�^���X�͒P��ł���K�v������܂�");
		engine_instance = this;
	}

	void Engine::Initialize()
	{
		
	}

	void Engine::Finalize()
	{
		
	}

	void Engine::Run()
	{
		
	}


}
