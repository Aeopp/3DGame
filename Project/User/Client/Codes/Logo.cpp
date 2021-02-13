//#include "..\\stdafx.h"
//#include "Logo.h"
//#include "ShaderManager.h"
//#include "ResourceSystem.h"
//#include "App.h"
//#include "ExportUtility.hpp"
//#include "StaticMesh.h"
//#include "imgui.h"
//#include "Vertexs.hpp"
//#include "TombStone.h"
//#include "Transform.h"
//
//void Logo::Initialize(const Vector3& Scale, const Vector3& Rotation, const Vector3& SpawnLocation)&
//{
//	Super::Initialize();
//
//	auto _Transform = AddComponent<Engine::Transform>();
//	_Transform->SetScale({ 1,1,1 });
//	_Transform->SetScale(Scale);
//	_Transform->SetRotation(Rotation);
//	_Transform->SetLocation(SpawnLocation);
//
//	auto _StaticMesh = AddComponent<Engine::StaticMesh>(L"Floor");
//
//	auto _Collision = AddComponent<Engine::Collision>
//		(Device, Engine::CollisionTag::Decorator, _Transform);
//
//	// 바운딩 박스.
//	{
//		Vector3  BoundingBoxMin{}, BoundingBoxMax{};
//		D3DXComputeBoundingBox(_StaticMesh->LocalVertexLocations->data(),
//			_StaticMesh->LocalVertexLocations->size(),
//			sizeof(Vector3), &BoundingBoxMin, &BoundingBoxMax);
//
//		_Collision->_Geometric = std::make_unique<Engine::OBB>
//			(BoundingBoxMin, BoundingBoxMax);
//
//		static_cast<Engine::OBB* const> (_Collision->_Geometric.get())->MakeDebugCollisionBox(Device);
//	}
//
//	RenderInterface::SetUpCullingInformation(
//		_Collision->_Geometric->LocalSphere,
//		_Transform);
//
//	RenderInterface::bCullingOn = true;
//
//	// 바운딩 스피어
//	{
//		/*Vector3 BoundingSphereCenter;
//		float BoundingSphereRadius;
//		D3DXComputeBoundingSphere(_StaticMesh->GetVertexLocations().data(), _StaticMesh->GetVertexLocations
//	.size(),
//			sizeof(Vector3), &BoundingSphereCenter, &BoundingSphereRadius);
//
//		_Collision->_Geometric = std::make_unique<Engine::GSphere>(BoundingSphereRadius,
//	undingSphereCenter);
//		static_cast<Engine::GSphere* const>(_Collision->_Geometric.get())->MakeDebugCollisionSphere
//	evice);*/
//	}
//
//	_Collision->RefCollisionables().insert(
//		{
//			Engine::CollisionTag::Decorator
//		});
//
//	_Collision->RefPushCollisionables().insert(
//		{
//			  Engine::CollisionTag::Decorator
//
//		});
//}
//
//void Logo::PrototypeInitialize(IDirect3DDevice9* const Device, const Engine::RenderInterface::Group _Group)&
//{
//	Super::PrototypeInitialize(Device, _Group);
//	this->Device = Device;
//
//	auto _StaticMeshProto = std::make_shared<Engine::StaticMesh>();
//
//	_StaticMeshProto->Load<Vertex::LocationNormal>(Device,
//		App::ResourcePath / L"Mesh" / L"StaticMesh" / L"SnowTerrain" / L"",
//		L"SnowTerrain.dae", L"Floor");
//
//	RefResourceSys().InsertAny<decltype(_StaticMeshProto)>(L"Floor", _StaticMeshProto);
//
//}
//
//void Logo::Event()&
//{
//	Super::Event();
//}
//
//void Logo::Render()&
//{
//	/*Super::Render();
//	const Matrix& World = GetComponent<Engine::Transform>()->
//		Device->SetTransform(D3DTS_WORLD, &World);
//	auto _StaticMesh = GetComponent<Engine::StaticMesh>();
//	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME)
//		_StaticMesh->Render();
//	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);*/
//}
//
//void Logo::Update(const float DeltaTime)&
//{
//	Super::Update(DeltaTime);
//}
