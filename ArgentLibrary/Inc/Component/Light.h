#pragma once
#include <DirectXMath.h>

#include "Component.h"

namespace argent::component
{
	//TODO ×‚©‚­•ª‚¯‚Ü‚µ‚å‚¤
	/**
	 * \brief ƒ‰ƒCƒgClass
	 * Directional, Point, Spot‚È‚Ç
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

