#include "Timer.h"
#include <sstream>
#include <iostream>

void Engine::Timer::Initialize(
	const float DeltaMax,
	std::function<void()> ApplicationEvent,
	std::function<void(const  float)> ApplicationUpdate,
	std::function<void()> ApplicationRender,
	std::function<void()> ApplicationLastEvent)
{
	using namespace std::chrono_literals;

	StartTime=PrevTime = std::chrono::high_resolution_clock::now();
	this->DeltaMax = DeltaMax;
	this->ApplicationEvent = std::move(ApplicationEvent);
	this->ApplicationUpdate = std::move(ApplicationUpdate);
	this->ApplicationLastEvent = std::move(ApplicationLastEvent);
	this->ApplicationRender = std::move(ApplicationRender);
}

void Engine::Timer::Update()
{
	using namespace std::chrono_literals;

	const std::chrono::time_point 
		CurrentTime = std::chrono::high_resolution_clock::now();

	const std::chrono::duration<float, std::ratio<1, 1000>> 
		Delta = CurrentTime - PrevTime;

	PrevTime = CurrentTime;
	SecCheck += Delta;

	DeltaTime = (Delta.count() * 0.001f) * TimeScale;
	Tick = DeltaTime;
	float Accumulator = DeltaTime;

	if (ApplicationEvent)
		ApplicationEvent();

	while (Accumulator > DeltaMax)
	{
		Accumulator -= DeltaMax;
		_T +=  DeltaTime = DeltaMax;

		NotificationCheck();

		if (ApplicationUpdate)
			ApplicationUpdate(GetDelta());
	}

	_T += DeltaTime = Accumulator;

	NotificationCheck();

	if (ApplicationUpdate)
		 ApplicationUpdate(GetDelta());

	if (SecCheck >= 1000ms)
	{
		FPS = _FPSCount;
		_FPSCount = 0;
		SecCheck -= 1000ms;
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

void Engine::Timer::RenderFPS() & noexcept
{
	if (!bTimeInfoRender)return;

	static constexpr float _10Billion = 1'000'000'000.f;

	const auto TimePassedNanoUnit =
		std::chrono::high_resolution_clock::now() - StartTime;
	const float PlayTime = TimePassedNanoUnit.count() /_10Billion;
	std::wstringstream StrInfo;

		StrInfo           << std::endl <<
		L" FPS : "        << FPS       << std::endl  <<
		L" DeltaTime : "  << DeltaTime << std::endl  <<
		L" TimeScale : "  << TimeScale << std::endl  <<
		L" InGameTime : " << _T        << std::endl  <<
		L" PlayTime : "   << PlayTime  << std::endl  ;

		Information = StrInfo.str(); 

		//std::wcout<< Information << std::endl;
}


