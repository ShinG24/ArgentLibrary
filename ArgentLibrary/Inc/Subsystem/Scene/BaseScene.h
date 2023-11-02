#pragma once

#include <DirectXMath.h>

#include <string>

namespace argent::scene
{
	/**
	 * \brief シーン基底クラス
	 */
	class BaseScene
	{
	public:

		BaseScene() = default;
		virtual ~BaseScene() = default;

		BaseScene(const BaseScene&) = delete;
		BaseScene(const BaseScene&&) = delete;
		BaseScene& operator=(const BaseScene&) = delete;
		BaseScene& operator=(const BaseScene&&) = delete;

		/**
		 * \brief 初期化関数
		 * このシーンがCurrent Sceneとして設定されて最初に呼ばれる
		 * この関数は上記の条件を満たすたびに呼ばれる
		 */
		virtual void Awake()
		{
			int i = 0;
		}

		/**
		 * \brief 終了関数
		 * このシーンがCurren Sceneではなくなったときに最後に呼ばれる
		 * この関数は上記の条件を満たすたびに呼ばれる
		 */
		virtual void Shutdown() {}

		/**
		 * \brief 更新関数
		 * このシーンがCurrent Sceneである限り、毎フレーム一回呼ばれる
		 * この関数は必ずRenderよりも前で呼ばれる
		 */
		virtual void Update() {}

		/**
		 * \brief 描画関数
		 * このシーンがCurrent Sceneである限り毎フレーム一回呼ばれる
		 * この関数はUpdate()より後に呼ばれ、OnGui()より先に呼ばれる
		 */
		virtual	void Render() {}

		/**
		 * \brief Gui上に描画する関数
		 */
		virtual void OnGui() {}

		std::string GetName() const { return name_; }

		//TODO 仮の実装　ユーザーが触る必要のない形に変える
		virtual DirectX::XMFLOAT3 GetCameraPosition() const = 0;
		virtual DirectX::XMFLOAT4X4 GetViewMatrix() const = 0;
		virtual DirectX::XMFLOAT4X4 GetProjectionMatrix() const = 0;
		virtual DirectX::XMFLOAT3 GetLightDirection() const = 0;

	protected:

		std::string name_ ;
	};
}

