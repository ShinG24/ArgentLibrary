#pragma once

#include <DirectXMath.h>

#include <memory>

#include "Subsystem/Subsystem.h"


namespace argent::graphics::dx12
{
	class ConstantBuffer;
}

namespace argent::graphics
{
	struct GraphicsContext;
	struct RenderContext;

	class Raytracer;

	/**
	 * \brief 描画マネージャ　描画管理を行う
	 * レイトレーシングを用いるかどうかもこいつで切り替える
	 */
	class RenderingManager final : public Subsystem
	{
	public:

		struct SceneConstant
		{
			DirectX::XMFLOAT4 camera_position_;
			DirectX::XMFLOAT4X4 view_matrix_;
			DirectX::XMFLOAT4X4 projection_matrix_;
			DirectX::XMFLOAT4X4 view_projection_matrix_;
			DirectX::XMFLOAT4X4 inv_view_projection_matrix_;
			DirectX::XMFLOAT4 light_direction_;
		};

	public:

		RenderingManager() = default;
		~RenderingManager() override = default;

		RenderingManager(const RenderingManager&) = delete;
		RenderingManager(const RenderingManager&&) = delete;
		RenderingManager& operator=(const RenderingManager&) = delete;
		RenderingManager& operator=(const RenderingManager&&) = delete;

		/**
		 * \brief 初期化
		 * シーンコンスタントバッファの作成
		 */
		void Awake() override;

		/**
		 * \brief 終了処理
		 */
		void Shutdown() override;

		void OnGui();

		/**
		 * \brief 開始命令　
		 * 描画に必要なデータ（カメラ、ライトなど）の情報を集める。
		 */
		void FrameBegin(const RenderContext* render_context, const SceneConstant& scene_data);

		/**
		 * \brief 描画終了
		 */
		void FrameEnd() const;

		/**
		 * \brief レイトレーシングの実行
		 * \param render_context RenderContext
		 * \param graphics_context Graphics Context
		 */
		void OnRaytrace(const RenderContext* render_context, const GraphicsContext* graphics_context) const;

		/**
		 * \brief レイトレモードがオンかどうか
		 * \return Is On Raytracing Mode
		 */
		bool IsRaytracing() const { return on_raytrace_; }

	private:

		std::unique_ptr<dx12::ConstantBuffer> scene_constant_buffer_;
		SceneConstant scene_data_{};
		bool on_raytrace_{ true };

		//TODO いずれまともな形にする
		std::unique_ptr<Raytracer> raytracer_;
	};
}

