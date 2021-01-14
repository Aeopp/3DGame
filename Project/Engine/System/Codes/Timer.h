#pragma once
#include "DllHelper.h"
#include "SingletonInterface.h"
#include <functional>
#include <chrono>
#include <algorithm>
#include "TypeAlias.h"

namespace Engine
{
	class DLL_DECL Timer :public SingletonInterface<Timer>
	{
	public:
		using NotifyEventType = std::function<bool()>;
		// ������ ���� ���� �Է����ּ���.
		void Initialize(
			const float DeltaMax,
			std::function<void()> ApplicationEvent,
			std::function<void(const  float)> ApplicationUpdate,
			std::function<void()> ApplicationRender,
			std::function<void()> ApplicationLastEvent
			);
		void Update();
		// ���� �ð� , �ݺ� �ֱ� , ���� �ð� , �̺�Ʈ
		// ��ȯ�� true �� ���� �ð� ��� ���� �̺�Ʈ ����
		// false �� ��� ����
		void TimerRegist(float initial, float Repeat, float End,
			NotifyEventType _NotifyEvent/*����ǥ���� or std::bind use*/);

		// ��ϵ� �ݹ� �Լ��鿡�� �����ϰ���� Ÿ�ֿ̹� ȣ���Ͽ��ּ���.
		void NotificationCheck()&;
	public:
		void TimeInfoRenderToggle()&;
		float GetDelta();
		float GetT();
		void SetDelta(const float DeltaTime)&;
		void SetTimeScale(const float TimeScale)&;
		void SetT(const float GetT);
	public:
		std::chrono::time_point<std::chrono::high_resolution_clock> PrevTime{};
	private:
		std::chrono::duration<float, std::ratio<1, 1000>> SecCheck{};
		std::chrono::time_point<std::chrono::high_resolution_clock> StartTime{};
		std::chrono::steady_clock::time_point CurrentTime{ std::chrono::high_resolution_clock::now() };
		float DeltaMax = 1.f/20.f;
	private:
		std::function<void()> ApplicationEvent;
		std::function<void(const  float)> ApplicationUpdate;
		std::function<void()> ApplicationRender;
		std::function<void()> ApplicationLastEvent;
	private:
		bool bTimeInfoRender = true;
		uint32 _FPSCount{ 0 };
		float DeltaTime{ 0.f };
		float _T{ 0.f };
		float TimeScale{ 1.f };
		uint32 FPS{ 0 };
		// �̺�Ʈ ���ۿ��� , ���� �ð� , �ݺ� �ֱ� , ���� �ð� , ���� ���� �ð�
		std::list<std::tuple<bool, float, float, float, float, float, NotifyEventType>> _NotifyEvents;
	private:
		void RenderFPS()const& noexcept;
	};
};

inline void Engine::Timer::TimeInfoRenderToggle()&
{
	bTimeInfoRender = !bTimeInfoRender;
}

inline float Engine::Timer::GetDelta()
{
	return DeltaTime;
}

inline float Engine::Timer::GetT()
{
	return _T;
}


inline void Engine::Timer::SetDelta(const float DeltaTime)&
{
	this->DeltaTime = std::clamp(DeltaTime * TimeScale, 0.f, (std::numeric_limits<float>::max)());
}

inline void Engine::Timer::SetTimeScale(const float TimeScale)&
{
	this->TimeScale = std::clamp(TimeScale, 0.f, (std::numeric_limits<float>::max)());
}

inline void Engine::Timer::SetT(const float GetT)
{
	this->_T = std::clamp(GetT, 0.f, (std::numeric_limits<float>::max)());
}

