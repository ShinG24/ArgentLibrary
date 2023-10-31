#pragma once

namespace argent
{
	class Subsystem
	{
	public:

		Subsystem() = default;
		virtual ~Subsystem() = default;

		Subsystem(const Subsystem&) = delete;
		Subsystem(const Subsystem&&) = delete;
		Subsystem& operator=(const Subsystem&) = delete;
		Subsystem& operator=(const Subsystem&&) = delete;

		virtual void Awake() {}
		virtual void Shutdown() {}

	};
}