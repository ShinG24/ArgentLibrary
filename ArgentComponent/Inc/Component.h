#pragma once

namespace argent::component
{
	/**
	 * \brief すべてのコンポーネントの基底クラス
	 * GameObjectが存在しないため（今のところ作るつもりもあまりないため）
	 * これといった機能は持っていない。
	 * 今後、コンポーネント指向に切り替える時に備えて継承するようにする
	 */
	class Component
	{
	public:
		Component() = default;
		virtual ~Component() = default;

		Component(const Component&) = delete;
		Component(const Component&&) = delete;
		Component operator=(const Component&) = delete;
		Component operator=(const Component&&) = delete;


		virtual void Awake() {}
		virtual void Start() {}

		virtual void Update() {}

		virtual void Shutdown() {}

		virtual void OnGui() {}
	private:
	};
}

