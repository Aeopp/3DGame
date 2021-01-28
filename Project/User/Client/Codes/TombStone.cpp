#include "..\\stdafx.h"
#include "TombStone.h"
#include "Transform.h"
#include "StaticMesh.h"

#include <iostream>
#include "StaticMesh.h"
#include "Collision.h"
#include "CollisionSystem.h"
#include "Controller.h"
#include "ExportUtility.hpp"
#include "dinput.h"
#include "imgui.h"
#include "Vertexs.hpp"
#include "ResourceSystem.h"
#include "App.h"

static uint32 TestID = 0u;
static bool bTestCollision = false;
void TombStone::Initialize(const Vector3& SpawnLocation , const Vector3& Rotation)&
{
	Super::Initialize();
	_TestID = TestID++;

	auto _Transform =AddComponent<Engine::Transform>();
	_Transform->SetScale({ 1,1,1 });
	_Transform->SetRotation(Rotation);
	_Transform->SetLocation(SpawnLocation);

	auto _StaticMesh =AddComponent<Engine::StaticMesh>(Device,L"TombStone");

	auto _Collision = AddComponent<Engine::Collision>
		(Device, Engine::CollisionTag::Decorator, _Transform);

	if(true)
	{
		Vector3  BoundingBoxMin{}, BoundingBoxMax{};
		D3DXComputeBoundingBox(_StaticMesh->GetVertexLocations().data(),
			_StaticMesh->GetVertexLocations().size(),
			sizeof(Vector3), &BoundingBoxMin, &BoundingBoxMax);

		_Collision->_Geometric = std::make_unique<Engine::OBB>
										(BoundingBoxMin, BoundingBoxMax);

		static_cast<Engine::OBB* const> (_Collision->_Geometric.get())->MakeDebugCollisionBox(Device);
	}
	else
	{
		Vector3 BoundingSphereCenter;
		float BoundingSphereRadius;
		D3DXComputeBoundingSphere(_StaticMesh->GetVertexLocations().data(), _StaticMesh->GetVertexLocations().size(),
			sizeof(Vector3), &BoundingSphereCenter, &BoundingSphereRadius);

		_Collision->_Geometric = std::make_unique<Engine::GSphere>(BoundingSphereRadius, BoundingSphereCenter);
		static_cast<Engine::GSphere* const>(_Collision->_Geometric.get())->MakeDebugCollisionSphere(Device);
	}

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

void TombStone::Event()&
{
	Super::Event();
	ImGui::Begin("CollisionTest");
	const std::string Msg = bTestCollision ? "Overlapped": "NoOverlapped" ;
	ImGui::Text(Msg.c_str());
	ImGui::End();
}

void TombStone::Render()&
{
	Super::Render();
	const Matrix& World = GetComponent<Engine::Transform>()->UpdateWorld();
	Device->SetTransform(D3DTS_WORLD, &World);
	auto _StaticMesh = GetComponent<Engine::StaticMesh>();
	Matrix View, Proj;
	Device->GetTransform(D3DTS_VIEW, &View);
	Device->GetTransform(D3DTS_PROJECTION, &Proj);
	IDirect3DVertexBuffer9* VertexBuffer{ nullptr }; 
	_StaticMesh->GetMesh()->GetVertexBuffer(&VertexBuffer);
	uint8* VertexBufferPtr{ nullptr }; 
	VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBufferPtr), NULL);

	for (uint32 Idx = 0u; Idx < _StaticMesh->GetMesh()->GetNumVertices(); ++Idx)
	{
		Vector3* const VertexLocation = 
			reinterpret_cast<Vector3*const>(  (VertexBufferPtr + (Idx * _StaticMesh->GetMesh()->GetNumBytesPerVertex()))); 
		Vector4 VectexLocationVec4 = {
		VertexLocation->x , 
		VertexLocation->y ,
		VertexLocation->z ,
		1.f };

		if (Idx == 10 && _TestID==0u )
		{
			int Debug = 0;
		}
		Matrix WorldViewProj = World * View * Proj;
		D3DXVec4Transform(&VectexLocationVec4, &VectexLocationVec4, &WorldViewProj);

		if (Idx ==10 && _TestID == 0u)
		{
			int Debug = 0;
		}
		


	}
	VertexBuffer->Unlock();
}

void TombStone::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
	bTestCollision = false;

	if (_TestID == 1u )
	{
		auto& Control = RefControl();
		auto _Transform = GetComponent<Engine::Transform>();
		static constexpr float Speed = 10.f;
	/*	_Transform->Rotate({ 0,1,0 }, 3.14f * 1 * DeltaTime);
		_Transform->Rotate({ 1,0,0 }, 3.14f * 1 * DeltaTime);
		_Transform->Rotate({ 0,0,1 }, 3.14f * 1 * DeltaTime);*/

		if (Control.IsPressing(DIK_UP))
		{
			_Transform->Move({ 0,0,1 }, DeltaTime, Speed);
		}
		if (Control.IsPressing(DIK_DOWN))
		{
			_Transform->Move({ 0,0,1 },DeltaTime, -Speed);
		}
		if (Control.IsPressing(DIK_LEFT))
		{
			_Transform->Move({1,0,0} , DeltaTime, -Speed);
		}
		if (Control.IsPressing(DIK_RIGHT))
		{
			_Transform->Move({1,0,0},DeltaTime, Speed);
		}
		if (Control.IsPressing(DIK_PGUP))
		{
			_Transform->Move({ 0,1,0 },DeltaTime, Speed);
		}
		if (Control.IsPressing(DIK_PGDN))
		{
			_Transform->Move({ 0,1,0 },  DeltaTime, -Speed);
		}
	}

	if (_TestID == 0u)
	{
		auto& Control = RefControl();
		auto _Transform = GetComponent<Engine::Transform>();
		static constexpr float Speed = 10.f;
		/*	_Transform->Rotate({ 0,1,0 }, 3.14f * 1 * DeltaTime);
			_Transform->Rotate({ 1,0,0 }, 3.14f * 1 * DeltaTime);
			_Transform->Rotate({ 0,0,1 }, 3.14f * 1 * DeltaTime);*/

		if (Control.IsPressing(DIK_R))
		{
			_Transform->RotateYaw(Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_T))
		{
			_Transform->RotateYaw(-Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_F))
		{
			_Transform->RotatePitch(Speed, DeltaTime); 
		}
		if (Control.IsPressing(DIK_G))
		{
			_Transform->RotatePitch(-Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_V))
		{
			_Transform->RotateRoll(Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_B))
		{
			_Transform->RotateRoll(-Speed, DeltaTime);
		}
	}


	if (_TestID == 1u)
	{
		auto& Control = RefControl();
		auto _Transform = GetComponent<Engine::Transform>();
		static constexpr float Speed = 10.f;
		/*	_Transform->Rotate({ 0,1,0 }, 3.14f * 1 * DeltaTime);
			_Transform->Rotate({ 1,0,0 }, 3.14f * 1 * DeltaTime);
			_Transform->Rotate({ 0,0,1 }, 3.14f * 1 * DeltaTime);*/

		if (Control.IsPressing(DIK_Y))
		{
			_Transform->RotateYaw(Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_U))
		{
			_Transform->RotateYaw(-Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_H))
		{
			_Transform->RotatePitch(Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_J))
		{
			_Transform->RotatePitch(-Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_N))
		{
			_Transform->RotateRoll(Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_M))
		{
			_Transform->RotateRoll(-Speed, DeltaTime);
		}
	}


}

void TombStone::HitNotify(Object* const Target, const Vector3 PushDir,
	const float CrossAreaScale)&
{
	Super::HitNotify(Target, PushDir, CrossAreaScale);
	bTestCollision = true;
	std::cout << "面倒吝" << std::endl;
};

void TombStone::HitBegin(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	Super::HitBegin(Target, PushDir, CrossAreaScale);
	std::cout << "面倒Start" << std::endl;
};

void TombStone::HitEnd(Object* const Target)&
{
	Super::HitEnd(Target);
	std::cout << "面倒场!" << std::endl;

};

