#include "..\\stdafx.h"
#include "TombStone.h"
#include "Transform.h"
#include "StaticMesh.h"
#include "Collision.h"
#include "CollisionSystem.h"

void TombStone::Initialize(const Vector3& SpawnLocation)&
{
	Super::Initialize();

	auto _Transform =AddComponent<Engine::Transform>();

	AddComponent<Engine::StaticMesh>(Device,L"TombStone");
	_Transform->SetLocation(SpawnLocation);

	auto _Collision =AddComponent<Engine::Collision>
		(Engine::CollisionTag::Decorator,_Transform);

	_Collision->_Geometric = std::make_unique<Engine::AABB>
		(Vector3{-1,-1,-1},Vector3{1,1,1});

	_Collision->RefCollisionables().insert(
		{
			Engine::CollisionTag::Decorator
		});

	_Collision->RefPushCollisionables().insert(
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
	Device->SetTransform(D3DTS_WORLD, &GetComponent<Engine::Transform>()->UpdateWorld());
	GetComponent<Engine::StaticMesh>()->Render();
}

void TombStone::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
}
