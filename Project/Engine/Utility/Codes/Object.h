#pragma once

namespace Engine
{
	class Object abstract
	{
	public:
		virtual void Update(const float DeltaTime)&;
		virtual void LateUpdate(const float DeltaTime)&;
	protected:
		virtual ~Object() noexcept = default;
	public:
		inline bool IsPendingKill()const& { return bPendingKill; };
		inline void Kill()& { bPendingKill = true; };
	private:
		bool bPendingKill{ false };
	};
};



