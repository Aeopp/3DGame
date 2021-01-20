#include "..\\stdafx.h"
#include "TombStone.h"
#include "Transform.h"
#include "StaticMesh.h"

void TombStone::Initialize()&
{
	Super::Initialize();

	AddComponent<Engine::Transform>();
	AddComponent<Engine::StaticMesh>(Device,L"TombStone");
}

void TombStone::PrototypeInitialize(IDirect3DDevice9* const Device,
						const Engine::RenderInterface::Group _Group)&
{
	Super::PrototypeInitialize(Device,_Group);
	this->Device = Device;
}

void TombStone::Render()&
{
	Super::Render();
	auto _StaticMesh = GetComponent<Engine::StaticMesh>();
	_StaticMesh->Render();
}

void TombStone::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
}
