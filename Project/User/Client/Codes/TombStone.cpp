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

	auto _StaticMesh =AddComponent<Engine::StaticMesh>(Device,L"TombStone");

	ID3DXMesh* Mesh = _StaticMesh->GetMesh();
	uint8* VertexBufferPtr{ nullptr };
	Vector3  BoundingBoxMin{}, BoundingBoxMax{};
	Mesh->LockVertexBuffer(0, (void**)&VertexBufferPtr);

	D3DXComputeBoundingBox((Vector3*)(VertexBufferPtr), Mesh->GetNumVertices(),
		Mesh->GetNumBytesPerVertex(), &BoundingBoxMin, &BoundingBoxMax);

	_Transform->SetLocation(SpawnLocation);

	auto _Collision =AddComponent<Engine::Collision>
		(Engine::CollisionTag::Decorator,_Transform);

	_Collision->_Geometric = std::make_unique<Engine::AABB>
						(BoundingBoxMin, BoundingBoxMax);

	_Collision->RefCollisionables().insert(
		{
			Engine::CollisionTag::Decorator
		});

	_Collision->RefPushCollisionables().insert(
		{
		//	Engine::CollisionTag::Decorator
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
	const Matrix& World = GetComponent<Engine::Transform>()->UpdateWorld();
	Device->SetTransform(D3DTS_WORLD, &World);
	GetComponent<Engine::StaticMesh>()->Render();
}

void TombStone::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
}
