#include "..\\stdafx.h"
#include "StartScene.h"
#include "Management.h"
#include "HeightMap.h"
#include "EnemyLayer.h"
#include "ExportUtility.hpp"
#include "FMath.hpp"
#include "App.h"
#include "StaticLayer.h"
#include <vector>
#include <array>
#include <numbers>
#include "Layer.h"
#include <iostream>
#include "DynamicCamera.h"


struct Location3DUV
{
    Vector3 Location;
    Vector3 UV;
    static inline constexpr uint32_t FVF = D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0);
};

struct _16_t
{
    WORD _1, _2, _3;
    static inline constexpr D3DFORMAT Format = D3DFMT_INDEX16;
};

void StartScene::Initialize(IDirect3DDevice9* const Device)&
{
    Super::Initialize(Device);

	auto* _Control = &GetControl();
	
	GetProto().LoadPrototype<Engine::HeightMap>(L"Static");
	GetProto().LoadPrototype<Engine::DynamicCamera>(L"Static", Device ,App::Hwnd);

	GetManager().NewLayer<EnemyLayer>();
	GetManager().NewLayer<StaticLayer>();

	constexpr float Aspect = App::ClientSize<float>.first /							   App::ClientSize<float>.second;

	GetManager().NewObject<StaticLayer,Engine::DynamicCamera>(
		L"Static", L"Camera",
		FMath::PI/3.f,0.1f,1000.f, Aspect , 10.f, _Control );
	GetManager().NewObject<StaticLayer, Engine::DynamicCamera>(
		L"Static", L"Camera2",
		FMath::PI / 3.f, 0.1f, 1000.f, Aspect , 10.f, _Control);
	GetManager().NewObject<StaticLayer, Engine::DynamicCamera>(
		L"Static", L"Camera3",
		FMath::PI / 3.f, 0.1f, 1000.f, Aspect, 10.f, _Control);
	GetManager().NewObject<StaticLayer, Engine::DynamicCamera>(
		L"Static", L"Camera4",
		FMath::PI / 3.f, 0.1f, 1000.f, Aspect, 10.f, _Control);

	GetManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap",Engine::RenderInterface::Group::Enviroment);
	GetManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap2", Engine::RenderInterface::Group::Enviroment);
	GetManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap3", Engine::RenderInterface::Group::Enviroment);
	GetManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap4", Engine::RenderInterface::Group::Enviroment);
	GetManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap5", Engine::RenderInterface::Group::Enviroment);
};

void StartScene::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);

	auto _Map = GetManager().FindObject<EnemyLayer, Engine::HeightMap>(L"HeightMap2");
	auto _Layer = GetManager().FindLayer<EnemyLayer>();
	std::wcout << _Map->GetName() << std::endl;

	auto& LeyerMap = GetManager().RefLayers();
	
	for (auto& q : GetManager().FindObjects<EnemyLayer, Engine::HeightMap>())
	{
		std::wcout << q->GetName() << std::endl;
	}
	for (auto& q : GetManager().FindObjects<StaticLayer, Engine::DynamicCamera>())
	{
		std::wcout << q->GetName() << std::endl;
	}

	auto Objs2 =GetManager().RefObjects<EnemyLayer>();
}





