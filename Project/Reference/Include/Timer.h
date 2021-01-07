#pragma once
#include "DllHelper.h"
#include "SingletonInterface.h"
#include <functional>
#include <chrono>
#include <algorithm>

namespace Engine
{
	class DLL_DECL Timer :public SingletonInterface<Timer>
	{
	public:
		using NotifyEventType = std::function<bool()>;
		// ������ ���� ���� �Է����ּ���.
		void Initialize(
			const uint32_t LimitFrame,
			const std::chrono::milliseconds DeltaMax,
			std::function<void(const  float)> ApplicationUpdate,
			std::function<void(const  float)> ApplicationLateUpdate,
			std::function<void()> ApplicationRender);
		void Update();
		void Render();

		// ���� �ð� , �ݺ� �ֱ� , ���� �ð� , �̺�Ʈ
		// ��ȯ�� true �� ���� �ð� ��� ���� �̺�Ʈ ����
		// false �� ��� ����
		void TimerRegist(float initial, float Repeat, float End,
			NotifyEventType _NotifyEvent/*����ǥ���� or std::bind use*/);

		// ��ϵ� �ݹ� �Լ��鿡�� �����ϰ���� Ÿ�ֿ̹� ȣ���Ͽ��ּ���.
		void NotificationCheck()&;
	public:
		float GetDelta();
		float GetT();
		void SetDelta(const float DeltaTime)&;
		void SetTimeScale(const float TimeScale)&;
		void SetT(const float GetT);
	public:
		std::chrono::time_point<std::chrono::high_resolution_clock> PrevTime{};
	private:
		std::chrono::duration<float, std::ratio<1, 1000>> SecCheck{};
		std::chrono::duration<float, std::ratio<1, 1000>> LimitDelta{};
		std::chrono::duration<float, std::ratio<1, 1000>> CurrentDelta{};
		std::chrono::duration<float, std::ratio<1, 1000>> Accumulator{};
		std::chrono::steady_clock::time_point CurrentTime{ std::chrono::high_resolution_clock::now() };
		std::chrono::milliseconds DeltaMax = std::chrono::milliseconds(25u);
	private:
		std::function<void(const  float)> ApplicationUpdate;
		std::function<void(const float)> ApplicationLateUpdate;
		std::function<void()> ApplicationRender;
	private:
		bool bTimeInfoRender = false;
		uint32_t _FPSCount{ 0 };
		float DeltaTime{ 0.f };
		float _T{ 0.f };
		float TimeScale{ 1.f };
		uint32_t FPS{ 0 };
		// �̺�Ʈ ���ۿ��� , ���� �ð� , �ݺ� �ֱ� , ���� �ð� , ���� ���� �ð�
		std::list<std::tuple<bool, float, float, float, float, float, NotifyEventType>> _NotifyEvents;
	private:
		void RenderFPS()const& noexcept;
	};

};






float Engine::Timer::GetDelta()
{
	return DeltaTime;
}

float Engine::Timer::GetT()
{
	return _T;
}


void Engine::Timer::SetDelta(const float DeltaTime)&
{
	this->DeltaTime = std::clamp(DeltaTime * TimeScale, 0.f, (std::numeric_limits<float>::max)());
}

void Engine::Timer::SetTimeScale(const float TimeScale)&
{
	this->TimeScale = std::clamp(TimeScale, 0.f, (std::numeric_limits<float>::max)());
}

void Engine::Timer::SetT(const float GetT)
{
	this->_T = std::clamp(GetT, 0.f, (std::numeric_limits<float>::max)());
}

