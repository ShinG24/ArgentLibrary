#pragma once

namespace argent::graphics
{
	//�o�b�N�o�b�t�@��
	static constexpr int kNumBackBuffers = 3;

	/**
	 * \brief �A���C�����g�}�N��
	 * \param value �A���C�����g�������l
	 * \param alignment_size �A���C�����g�̒P��
	 */
	#define _ALIGNMENT_(value, alignment_size)	 (((value) + (alignment_size)-1) & ~((alignment_size)-1))
}
