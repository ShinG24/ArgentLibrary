#pragma once

#include "ArgentLibrary.h"

class DemoScene : public argent::scene::BaseScene
{
public:

	DemoScene() = default;

	void Awake() override;
	void Shutdown() override;

	void Update() override;
	void OnRender3D(const argent::graphics::RenderContext* render_context) override;
	void OnGui() override;

	DirectX::XMFLOAT3 GetCameraPosition() const override { return camera_.GetEye(); }
	DirectX::XMFLOAT4X4 GetViewMatrix() const override { return camera_.CalcViewMatrix(); };
	DirectX::XMFLOAT4X4 GetProjectionMatrix() const override { return camera_.CalcProjectionMatrix(); }
	DirectX::XMFLOAT3 GetLightDirection() const override { return directional_light_.GetDirection(); }

private:

	argent::component::Camera camera_;
	argent::component::Light directional_light_;

	float rotation_speed_ = 0.004f;
	float move_speed_ = 5.0f;

};

