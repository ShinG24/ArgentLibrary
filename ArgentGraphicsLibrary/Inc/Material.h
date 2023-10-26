#pragma once
#include "GameResource.h"

namespace argent::graphics
{
	class GraphicsContext;
}

namespace argent::graphics
{
	class GameResource;

	class Material : public GameResource
	{
	public:
		Material();

		/**
		 * \brief �R���X�g���N�^ �Ł[����std::move()�ňڍs�����邽�ߏ��L�����Ȃ��Ȃ邱�Ƃɒ���
		 * \param name Material name
		 */
		Material(std::string& name);
		~Material() override = default;

		/**
		 * \brief �e�N�X�`�����邢�̓R���X�^���g�o�b�t�@�̏������֐�
		 * �}�e���A���͉\�Ȍ��蔖���\���ɂ��邽�߁A�����ł͉������Ȃ��悤�ɂ���
		 * \param graphics_context GraphicsContext
		 */
		virtual void Awake(const GraphicsContext* graphics_context) = 0;
	private:
		
	};
}
