#include "..\\stdafx.h"
#include "TombStone.h"
#include "Transform.h"
#include "StaticMesh.h"

void TombStone::Initialize()&
{
	Super::Initialize();
	AddComponent<Engine::Transform>();
	AddComponent<Engine::StaticMesh>(Device,Engine::RenderInterface::Group::Enviroment,
		L"TombStone");
}

void TombStone::PrototypeInitialize(IDirect3DDevice9* const Device)&
{
	Super::PrototypeInitialize();
	this->Device = Device;
}

void TombStone::Update(const float DeltaTime)&
{

}
