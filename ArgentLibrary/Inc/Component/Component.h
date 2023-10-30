#pragma once

namespace argent::component
{
	/**
	 * \brief ���ׂẴR���|�[�l���g�̊��N���X
	 * GameObject�����݂��Ȃ����߁i���̂Ƃ������������܂�Ȃ����߁j
	 * ����Ƃ������@�\�͎����Ă��Ȃ��B
	 * ����A�R���|�[�l���g�w���ɐ؂�ւ��鎞�ɔ����Čp������悤�ɂ���
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

