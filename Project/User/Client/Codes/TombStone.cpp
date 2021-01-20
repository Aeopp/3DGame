#include "..\\stdafx.h"
#include "TombStone.h"
#include "Transform.h"
#include "StaticMesh.h"
#include "Collision.h"
#include "CollisionSystem.h"


void TombStone::Initialize()&
{
	Super::Initialize();

	AddComponent<Engine::Transform>();
	AddComponent<Engine::StaticMesh>(Device,L"TombStone");

	auto _Collision =AddComponent<Engine::Collision>
		(Engine::CollisionTag::Decorator);

	_Collision->_Geometric = std::make_unique<Engine::AABB>
		(Vector3{-1,-1,-1},Vector3{1,1,1});

	_Collision->RefCollisionables().insert(
		{
			Engine::CollisionTag::Decorator
		});
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
