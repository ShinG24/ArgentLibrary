#pragma once
#include <DirectXMath.h>

#include "Component.h"

namespace argent::component
{
	//TODO 細かく分けましょう
	/**
	 * \brief ライトClass
	 * Directional, Point, Spotなど
	 */
	class Light : public  Component
	{
	public:
		enum class Type
		{
			Directional,
		};
	public:
		Light();
		virtual ~Light() override = default;

		DirectX::XMFLOAT3 GetDirection() const { return direction_; }

	private:
		DirectX::XMFLOAT3 direction_;
		Type type_;
	};
	
}

