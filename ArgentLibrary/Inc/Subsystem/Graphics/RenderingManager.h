#pragma once

#include <DirectXMath.h>

#include <memory>

#include "Subsystem/Subsystem.h"


namespace argent::graphics::dx12
{
	class ConstantBuffer;
	class RootSignature;
	class GraphicsPipelineState;
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

		/**
		 * \brief 描画命令
		 * シーンの描画、ポストプロセス、UI描画
		 * \param render_context Render Context
		 */
		void Render(const RenderContext* render_context);

		void OnGui();

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

		std::shared_ptr<dx12::RootSignature> GetRasterGlobalRootSignature() const { return scene_constant_binding_signature_; }

	private:

		std::shared_ptr<dx12::RootSignature> scene_constant_binding_signature_;
		std::unique_ptr<dx12::ConstantBuffer> scene_constant_buffer_{};

		SceneConstant scene_data_{};
		bool on_raytrace_{ true };

		//TODO いずれまともな形にする
		std::unique_ptr<Raytracer> raytracer_{};
	};
}

