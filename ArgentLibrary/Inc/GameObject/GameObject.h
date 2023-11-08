#pragma once

#include <string>
#include <memory>
#include <vector>

#include "../Component/Component.h"

namespace argent
{
	class GameObject
	{
	public:

		GameObject();
		~GameObject() = default;

		void Awake();
		void Update();

		void OnGui();
	private:

		std::string name_;
		std::vector<std::unique_ptr<component::Component>> component_vec_;

	};
}

