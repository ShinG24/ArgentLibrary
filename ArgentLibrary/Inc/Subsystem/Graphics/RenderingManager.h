#pragma once

#include "Subsystem/Subsystem.h"

namespace argent::graphics
{
	/**
	 * \brief �`��}�l�[�W���@�`��Ǘ����s��
	 * ���C�g���[�V���O��p���邩�ǂ����������Ő؂�ւ���
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

