#include "Core/Engine.h"

#include "Core/SubsystemLocator.h"

namespace argent
{
	static Engine* engine_instance = nullptr;

	Engine::Engine()
	{
		if(engine_instance) _ASSERT_EXPR(false, L"Engineのインスタンスはすでに生成されています\n"
										  "インスタンスは単一である必要があります");
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
