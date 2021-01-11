#include "Timer.h"
#include <sstream>
#include <iostream>

void Engine::Timer::Initialize(const uint32_t LimitFrame,
	const std::chrono::milliseconds DeltaMax,
	std::function<void(const  float)> ApplicationUpdate,
	std::function<void()> ApplicationLastEvent,
	std::function<void()> ApplicationRender)
{
	using namespace std::chrono_literals;

	PrevTime = std::chrono::high_resolution_clock::now();
	Accumulator = 0ms;
	LimitDelta = 1000ms / (float)LimitFrame;
	this->DeltaMax = DeltaMax;

	this->ApplicationUpdate = std::move(ApplicationUpdate);
	this->ApplicationLastEvent = std::move(ApplicationLastEvent);
	this->ApplicationRender = std::move(ApplicationRender);
}

void Engine::Timer::Update()
{
	using namespace std::chrono_literals;

	std::chrono::time_point CurrentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float, std::ratio<1, 1000>> Delta = CurrentTime - PrevTime;

	PrevTime = CurrentTime;
	Accumulator += Delta;
	SecCheck += Delta;

	while (Accumulator >= LimitDelta)
	{
		DeltaTime = (LimitDelta.count() * 0.001f) * TimeScale;
		_T += DeltaTime;
		Accumulator -= LimitDelta;

		NotificationCheck();

		if (ApplicationUpdate)
			ApplicationUpdate(GetDelta());
	}

	if (SecCheck >= 1000ms)
	{
		FPS = _FPSCount;
		_FPSCount = 0;
		SecCheck = 0ms;
	}
	else
		++_FPSCount;

	if (ApplicationRender)
		ApplicationRender();
	RenderFPS();
	if (ApplicationLastEvent)
		ApplicationLastEvent();
}






void Engine::Timer::TimerRegist(float initial, float Repeat, float End,
	NotifyEventType _NotifyEvent)
{
	End = (std::max)(End, initial);
	_NotifyEvents.emplace_back(false, initial, Repeat, End, 0.f, initial, std::move(_NotifyEvent));
}

void Engine::Timer::NotificationCheck()&
{
	// 시작 시간 , 반복 주기 , 종료 시간 , 현재 측정 시간
	for (auto iter = std::begin(_NotifyEvents);
		iter != std::end(_NotifyEvents);)
	{
		auto& [bInit, Init, Repeat, End, CurrentDelta, initial, Event] = *iter;

		Init -= DeltaTime;
		if (Init > 0.f)continue;

		if (Init < 0.f && !bInit)
		{
			bInit = true;
			CurrentDelta = initial;
			continue;
		}

		CurrentDelta -= DeltaTime;
		End -= DeltaTime;
		bool IsNotifyEnd = false;

		if (CurrentDelta < 0.f)
		{
			IsNotifyEnd |= Event();
			CurrentDelta = Repeat;
		}

		IsNotifyEnd |= (End < 0.f);

		if (IsNotifyEnd)
		{
			iter = _NotifyEvents.erase(iter);
			continue;
		}

		++iter;
	}
};

void Engine::Timer::RenderFPS() const& noexcept
{
	if (!bTimeInfoRender)return;

	std::wstringstream StrInfo;
	StrInfo
		<< L"FPS : " << FPS <<
		L"DeltaTime : " << DeltaTime <<
		L"TimeScale : " << TimeScale <<
		L"PlayTime  : " << _T << std::endl;
}


