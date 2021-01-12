#include "stdafx.h"
#include "StartScene.h"
#include "Management.h"
#include "EnemyLayer.h"
#include "HeightMap.h"

void StartScene::Initialize(IDirect3DDevice9* const Device)&
{
	Super::Initialize(Device);

	//Engine::Management::Instance->NewObject<EnemyLayer, 
	//	Engine::HeightMap>									(L"HeightMap",Engine::RenderInterface::Group::Enviroment);
}

void StartScene::Update(const float DeltaTime)&
{


	Super::Update(DeltaTime);


}
