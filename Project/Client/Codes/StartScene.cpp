#include "..\\stdafx.h"
#include "StartScene.h"
#include "Management.h"
#include "HeightMap.h"
#include "EnemyLayer.h"
#include "ExportUtility.hpp"
#include "Controller.h"
#include "App.h"
#include "Layer.h"

void StartScene::Initialize(IDirect3DDevice9* const Device)&
{
    Super::Initialize(Device);

	GetManager().NewLayer<EnemyLayer>();
	GetManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Name", Engine::RenderInterface::Group::Enviroment);
};

void StartScene::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);

	auto _Map = GetManager().FindObject<EnemyLayer, Engine::HeightMap>(L"Name");
	
}
