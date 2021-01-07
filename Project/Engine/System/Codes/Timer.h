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
		// 프레임 제한 값을 입력해주세요.
		void Initialize(
			const uint32_t LimitFrame,
			const std::chrono::milliseconds DeltaMax,
			std::function<void(const  float)> ApplicationUpdate,
			std::function<void(const  float)> ApplicationLateUpdate,
			std::function<void()> ApplicationRender);
		void Update();
		void Render();

		// 시작 시간 , 반복 주기 , 종료 시간 , 이벤트
		// 반환값 true 시 종료 시간 상관 없이 이벤트 삭제
		// false 시 계속 유지
		void TimerRegist(float initial, float Repeat, float End,
			NotifyEventType _NotifyEvent/*람다표현식 or std::bind use*/);

		// 등록된 콜백 함수들에게 통지하고싶은 타이밍에 호출하여주세요.
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
		// 이벤트 시작여부 , 시작 시간 , 반복 주기 , 종료 시간 , 현재 측정 시간
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

