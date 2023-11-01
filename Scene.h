#pragma once
#include "Subsystem/Scene/BaseScene.h"

class DemoScene : public argent::scene::BaseScene
{
public:
	DemoScene() = default;


	void Awake() override;
	void Shutdown() override;

	void Update() override;
	void Render() override;

private:

};

