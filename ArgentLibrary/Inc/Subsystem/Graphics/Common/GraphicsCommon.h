#pragma once

//グラフィックス内で共通で使う奴らを置く場所

namespace argent::graphics
{
	//バックバッファ数
	static constexpr int kNumBackBuffers = 3;

	/**
	 * \brief アライメントマクロ
	 * \param value アライメントしたい値
	 * \param alignment_size アライメントの単位
	 */
	#define _ALIGNMENT_(value, alignment_size)	 (((value) + (alignment_size)-1) & ~((alignment_size)-1))
}
