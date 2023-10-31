#pragma once
#include "Subsystem/Scene/BaseScene.h"

#include <iostream>

class Scene : public argent::scene::BaseScene
{
public:
	Scene()
	{
		std::cout << "Create Scene";
	}
	//Scene();

	void Awake() override { std::cout << "awake";};
	void Shutdown() override;

	void Update() override;
	void Render() override;

private:

};

