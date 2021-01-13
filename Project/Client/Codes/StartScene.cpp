#include "..\\stdafx.h"
#include "StartScene.h"
#include "Management.h"
#include "HeightMap.h"
#include "EnemyLayer.h"
#include "ExportUtility.hpp"
#include "Controller.h"
#include "App.h"
#include "Layer.h"
#include <iostream>


void StartScene::Initialize(IDirect3DDevice9* const Device)&
{
    Super::Initialize(Device);

	GetProto().LoadPrototype <Engine::HeightMap>(L"Standard");

	GetManager().NewLayer<EnemyLayer>();

	auto _pptr = GetManager().NewObject<EnemyLayer, Engine::HeightMap>(
		L"Standard",L"Name", Engine::RenderInterface::Group::Enviroment);
};

void StartScene::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);

	auto _Map = GetManager().FindObject<EnemyLayer, Engine::HeightMap>(L"Name");
	auto _Layer = GetManager().FindLayer<EnemyLayer>();

	auto& LeyerMap = GetManager().RefLayers();
	auto Objs = GetManager().FindObjects<EnemyLayer, Engine::HeightMap>();
	auto Objs2 =GetManager().RefObjects<EnemyLayer>();

	if (GetControl().IsDown(DIK_LEFTCLICK))
	{	
		std::cout << "Ŭ��" << std::endl;
	}

	if (GetControl().IsPressing(DIK_LEFTCLICK))
	{	
		std::cout << "������" << std::endl;
	}
	if (GetControl().IsUp(DIK_LEFTCLICK))
	{
		//MessageBox(nullptr, L"��", L"��", MB_OK);
		std::cout << "��" << std::endl;
	}

	if (GetControl().IsNone(DIK_LEFTCLICK))
	{
		//std::cout << "��" << std::endl;
	}
}




