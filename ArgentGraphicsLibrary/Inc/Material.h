#pragma once
#include "GameResource.h"


namespace argent::graphics
{
	class GameResource;

	class Material : public GameResource
	{
	public:
		Material();
		~Material();

		void OnGui() override;
	private:
	};
}
