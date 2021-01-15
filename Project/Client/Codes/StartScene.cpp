#include "..\\stdafx.h"
#include "StartScene.h"
#include "Management.h"
#include "HeightMap.h"
#include "EnemyLayer.h"
#include "ExportUtility.hpp"
#include "ResourceSystem.h"
#include "FMath.hpp"
#include "App.h"
#include "StaticLayer.h"
#include <vector>
#include <array>
#include <numbers>
#include "Layer.h"
#include <iostream>
#include "DynamicCamera.h"
#include <d3d9.h>
#include <d3dx9.h>


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

void WINAPI
D3DXCreateTextureFrom(
	LPDIRECT3DDEVICE9         pDevice,
	LPCWSTR                   pSrcFile,
	LPDIRECT3DTEXTURE9* ppTexture)
{
	D3DXCreateTextureFromFile(pDevice, pSrcFile, ppTexture);
}

void StartScene::Initialize(IDirect3DDevice9* const Device)&
{
    Super::Initialize(Device);
	
	auto* _Control = &RefControl();
	IDirect3DTexture9* aas{ nullptr };
	/*Matrix w;
	Device->SetTransform(D3DTS_WORLD, &w);*/
	   const std::wstring ssss = L"..\\..\\Resource\\Texture\\Player0.jpg"; 
//	D3DXCreateTextureFromFile();
	//std::invoke(D3DXCreateTextureFrom, Device, ssss.c_str(), &aas);
	// D3DXCreateTextureFromFile(Device, ssss.c_str(), &aas);
	
	RefResourceSys().Create<IDirect3DTexture9>(
		L"SS",D3DXCreateTextureFromFile,Device,L"..\\..\\Resource\\Texture\\Player0.jpg",&aas);

	RefProto().LoadPrototype<Engine::HeightMap>(L"Static");
	RefProto().LoadPrototype<Engine::DynamicCamera>(L"Static", Device ,App::Hwnd);

	RefManager().NewLayer<EnemyLayer>();
	RefManager().NewLayer<StaticLayer>();

	constexpr float Aspect = App::ClientSize<float>.first /							   App::ClientSize<float>.second;

	RefManager().NewObject<StaticLayer,Engine::DynamicCamera>(
		L"Static", L"Camera",
		FMath::PI/3.f,0.1f,1000.f, Aspect , 10.f, _Control );
	RefManager().NewObject<StaticLayer, Engine::DynamicCamera>(
		L"Static", L"Camera2",
		FMath::PI / 3.f, 0.1f, 1000.f, Aspect , 10.f, _Control);
	RefManager().NewObject<StaticLayer, Engine::DynamicCamera>(
		L"Static", L"Camera3",
		FMath::PI / 3.f, 0.1f, 1000.f, Aspect, 10.f, _Control);
	RefManager().NewObject<StaticLayer, Engine::DynamicCamera>(
		L"Static", L"Camera4",
		FMath::PI / 3.f, 0.1f, 1000.f, Aspect, 10.f, _Control);

	RefManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap",Engine::RenderInterface::Group::Enviroment);
	RefManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap2", Engine::RenderInterface::Group::Enviroment);
	RefManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap3", Engine::RenderInterface::Group::Enviroment);
	RefManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap4", Engine::RenderInterface::Group::Enviroment);
	RefManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap5", Engine::RenderInterface::Group::Enviroment);
};

void StartScene::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);

	auto _Map = RefManager().FindObject<EnemyLayer, Engine::HeightMap>(L"HeightMap2");
	auto _Layer = RefManager().FindLayer<EnemyLayer>();
	std::wcout << _Map->GetName() << std::endl;

	auto& LeyerMap = RefManager().RefLayers();
	
	for (auto& q : RefManager().FindObjects<EnemyLayer, Engine::HeightMap>())
	{
		std::wcout << q->GetName() << std::endl;
	}
	for (auto& q : RefManager().FindObjects<StaticLayer, Engine::DynamicCamera>())
	{
		std::wcout << q->GetName() << std::endl;
	}

	auto Objs2 = RefManager().RefObjects<EnemyLayer>();
}





