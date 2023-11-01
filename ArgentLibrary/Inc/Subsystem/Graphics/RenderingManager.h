#pragma once

#include "Subsystem/Subsystem.h"

namespace argent::graphics
{
	/**
	 * \brief 描画マネージャ　描画管理を行う
	 * レイトレーシングを用いるかどうかもこいつで切り替える
	 */
	class RenderingManager final : public Subsystem
	{
	public:
		RenderingManager() = default;
		~RenderingManager() override = default;

		RenderingManager(const RenderingManager&) = delete;
		RenderingManager(const RenderingManager&&) = delete;
		RenderingManager& operator=(const RenderingManager&) = delete;
		RenderingManager& operator=(const RenderingManager&&) = delete;

		
	private:
		bool on_raytrace_;
	};
}

