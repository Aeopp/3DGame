#include "Geometric.h"
#include "FMath.hpp"
#include "imgui.h"
#include <iostream>
#include "Vertexs.hpp"
#include <string>
#include "ResourceSystem.h"


static uint32 DebugVertexBufferID = 0u;

Engine::AABB::AABB(const Vector3 LocalMin, const Vector3 LocalMax)
	: LocalMin{ LocalMin }, LocalMax{ LocalMax }
{}

Engine::Geometric::Type Engine::AABB::GetType() const&
{
	return Engine::Geometric::Type::AABB;
}

void Engine::AABB::MakeDebugCollisionBox(IDirect3DDevice9* const Device)&
{
	Device->CreateVertexBuffer(
		sizeof(Vertex::Location3DUV) * 8u,
		D3DUSAGE_WRITEONLY,
		Vertex::Location3DUV::FVF,
		D3DPOOL_MANAGED,
		&VertexBuffer, nullptr);

	auto& ResourceSys = ResourceSystem::Instance;

	Vertex::Location3DUV* VertexBufferPtr{ nullptr };
	VertexBuffer->Lock(0, 0, (void**)&VertexBufferPtr, NULL);

	// 전면
	VertexBufferPtr[0].Location = Vector3(LocalMin.x, LocalMax.y, LocalMin.z);
	VertexBufferPtr[0].UV = VertexBufferPtr[0].Location;

	VertexBufferPtr[1].Location = Vector3(LocalMax.x, LocalMax.y, LocalMin.z);
	VertexBufferPtr[1].UV = VertexBufferPtr[1].Location;

	VertexBufferPtr[2].Location = Vector3(LocalMax.x, LocalMin.y, LocalMin.z);
	VertexBufferPtr[2].UV = VertexBufferPtr[2].Location;

	VertexBufferPtr[3].Location = Vector3(LocalMin.x, LocalMin.y, LocalMin.z);
	VertexBufferPtr[3].UV = VertexBufferPtr[3].Location;

	// 후면
	VertexBufferPtr[4].Location = Vector3(LocalMin.x, LocalMax.y, LocalMax.z);
	VertexBufferPtr[4].UV = VertexBufferPtr[4].Location;

	VertexBufferPtr[5].Location = Vector3(LocalMax.x, LocalMax.y, LocalMax.z);
	VertexBufferPtr[5].UV = VertexBufferPtr[5].Location;

	VertexBufferPtr[6].Location = Vector3(LocalMax.x, LocalMin.y, LocalMax.z);
	VertexBufferPtr[6].UV = VertexBufferPtr[6].Location;

	VertexBufferPtr[7].Location = Vector3(LocalMin.x, LocalMin.y, LocalMax.z);
	VertexBufferPtr[7].UV = VertexBufferPtr[7].Location;

	VertexBuffer->Unlock();


	ResourceSys->Insert<IDirect3DVertexBuffer9>
		(L"VertexBuffer_Location3DUV_Cube_Collision_" + std::to_wstring(++DebugVertexBufferID), VertexBuffer);

	IndexBuffer = ResourceSys->Get<IDirect3DIndexBuffer9>(L"IndexBuffer_Cube");
	CollisionTexture = ResourceSys->Get<IDirect3DTexture9>(L"Texture_Collision");
	NoCollisionTexture = ResourceSys->Get<IDirect3DTexture9>(L"Texture_NoCollision");
}

void Engine::AABB::Update(const Vector3 Scale,
	const Vector3 Rotation,
	const Vector3 Location)&
{
	const Matrix ToWorld = FMath::WorldMatrix(Scale, { 0,0,0 }, Location);
	Min = FMath::Mul(LocalMin, ToWorld);
	Max = FMath::Mul(LocalMax, ToWorld);
};

void Engine::AABB::Render(IDirect3DDevice9* const Device , const bool bCurrentUpdateCollision)&
{
	IDirect3DTexture9* DebugTexture = bCurrentUpdateCollision ? CollisionTexture  : NoCollisionTexture;

	Device->SetTexture(0,DebugTexture);
	Device->SetStreamSource(0, VertexBuffer, 0, sizeof(Vertex::Location3DUV));
	Device->SetFVF(Vertex::Location3DUV::FVF);
	Device->SetIndices(IndexBuffer);
	Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);
}

bool Engine::AABB::IsCollision
(Geometric* const Rhs,
	Vector3& PushDir, float& CrossAreaScale)&
{
	switch (Rhs->GetType())
	{
	case Type::AABB:
		return IsCollisionAABB(Rhs, PushDir, CrossAreaScale);
		break;
	case Type::OBB:
		return IsCollisionOBB(Rhs, PushDir, CrossAreaScale);
		break;
	default:
		break;
	}

	return false;
}

bool Engine::AABB::IsCollisionAABB(
	Geometric* const Rhs,
	Vector3& PushDir,
	float& CrossAreaScale) const&
{
	auto RhsAABB = static_cast<AABB* const>(Rhs);

	if (RhsAABB->Min.x > Max.x || RhsAABB->Max.x < Min.x)
		return false;
	if (RhsAABB->Min.y > Max.y || RhsAABB->Max.y < Min.y)
		return false;
	if (RhsAABB->Min.z > Max.z || RhsAABB->Max.z < Min.z)
		return false;

	return true;
}

bool Engine::AABB::IsCollisionOBB(Geometric* const Rhs, Vector3& PushDir, float& CrossAreaScale) const&
{
	return false;
}

Engine::OBB::OBB(const Vector3 LocalMin, const Vector3 LocalMax)
	:
	LocalCenter{ (LocalMin + LocalMax) / 2.f } ,
	LocalPoints
   {
       Vector3(LocalMin.x, LocalMax.y, LocalMin.z) ,
       Vector3(LocalMax.x, LocalMax.y, LocalMin.z),
       Vector3(LocalMax.x, LocalMin.y, LocalMin.z),
       Vector3(LocalMin.x, LocalMin.y, LocalMin.z),
       Vector3(LocalMin.x, LocalMax.y, LocalMax.z),
       Vector3(LocalMax.x, LocalMax.y, LocalMax.z),
       Vector3(LocalMax.x, LocalMin.y, LocalMax.z),
       Vector3(LocalMin.x, LocalMin.y, LocalMax.z)
    } ,
	LocalFaceNormals 
	{
		FMath::GetNormalFromFace(LocalPoints[1],LocalPoints[5],LocalPoints[6]) ,
		FMath::GetNormalFromFace(LocalPoints[4],LocalPoints[5],LocalPoints[1]),
		FMath::GetNormalFromFace(LocalPoints[7],LocalPoints[6],LocalPoints[5])
	}
{}

void Engine::OBB::MakeDebugCollisionBox(IDirect3DDevice9* const Device)
{
	Device->CreateVertexBuffer(
		sizeof(Vertex::Location3DUV) * 8u,
		D3DUSAGE_WRITEONLY,
		Vertex::Location3DUV::FVF,
		D3DPOOL_MANAGED,
		&VertexBuffer, nullptr);

	auto& ResourceSys = ResourceSystem::Instance;

	Vertex::Location3DUV* VertexBufferPtr{ nullptr };
	VertexBuffer->Lock(0, 0, (void**)&VertexBufferPtr, NULL);

	for (size_t Idx = 0u; Idx < LocalPoints.size(); ++Idx)
	{
		VertexBufferPtr[Idx].UV = VertexBufferPtr[Idx].Location =  LocalPoints[Idx];
	}

	VertexBuffer->Unlock();

	ResourceSys->Insert<IDirect3DVertexBuffer9>
		(L"VertexBuffer_Location3DUV_Cube_Collision_" + std::to_wstring(++DebugVertexBufferID), VertexBuffer);

	IndexBuffer = ResourceSys->Get<IDirect3DIndexBuffer9>(L"IndexBuffer_Cube");
	CollisionTexture = ResourceSys->Get<IDirect3DTexture9>(L"Texture_Collision");
	NoCollisionTexture = ResourceSys->Get<IDirect3DTexture9>(L"Texture_NoCollision");
}


Engine::Geometric::Type Engine::OBB::GetType() const&
{
	return Engine::Geometric::Type::OBB;
};

void Engine::OBB::Update(const Vector3 Scale, const Vector3 Rotation, const Vector3 Location)&
{
	const Matrix ToWorld = FMath::WorldMatrix(Scale, Rotation, Location);
	WorldCenter = FMath::Mul(LocalCenter, ToWorld);
	std::transform(std::begin(LocalFaceNormals), std::end(LocalFaceNormals), std::begin(WorldFaceNormals),
		[ToWorld](const Vector3& LocalFaceNormal) {
			return FMath::MulNormal(LocalFaceNormal, ToWorld);
		});
	std::transform(std::begin(LocalPoints), std::end(LocalPoints), std::begin(WorldPoints),
		[ToWorld](const Vector3& LocalPoint) {
			return FMath::Mul(LocalPoint, ToWorld);
		});
}

void Engine::OBB::Render(IDirect3DDevice9* const Device , const bool bCurrentUpdateCollision)&
{
	IDirect3DTexture9* DebugTexture = bCurrentUpdateCollision ? CollisionTexture  : NoCollisionTexture;
	Device->SetTexture(0, DebugTexture);
	Device->SetStreamSource(0, VertexBuffer, 0, sizeof(Vertex::Location3DUV));
	Device->SetFVF(Vertex::Location3DUV::FVF);
	Device->SetIndices(IndexBuffer);
	Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);
}

bool Engine::OBB::IsCollision(Geometric* const Rhs, Vector3& PushDir, float& CrossAreaScale)&
{
	switch (Rhs->GetType())
	{
	case Type::AABB:
		return IsCollisionAABB(Rhs, PushDir, CrossAreaScale);
		break;
	case Type::OBB:
		return IsCollisionOBB(Rhs, PushDir, CrossAreaScale);
		break;
	default:
		break;
	}

	return false;
}

bool Engine::OBB::IsCollisionOBB(Geometric* const Rhs, Vector3& PushDir, float& CrossAreaScale) const&
{
	auto RhsOBB = static_cast<OBB* const>(Rhs);

	// 충돌 검사하려는 다면체를 A , B라 한다.
	// 다면체가 충돌하지 않는다면 반드시 분리축이 하나 이상 존재한다.
	// 분리축은 아래 세가지 조건중 하나에 반드시 수직이다.
	// 1. A를 구성하는 면들중 하나 이상의 노말벡터, 2. B를 구성하는 면들중 하나 이상의 노말벡터, 
	// 3. A의 선분들중 하나 이상과 B의 선분들중 하나 이상이 존재하는 평면의 수직인 노말벡터
	// 다면체가 박스일 경우 면의 노말 6개들중 2쌍씩 서로 같은 축을 공유하므로 검사하려는 노말을 3개로 최적화 할 수 있다.
	// 다면체가 박스일 경우 4개의 선분들이 모두 평행하므로 (같은 방향,축이므로) 12/4 = 3 검사해야하는 축을 3개로 최적화 할 수 있다.
	// 두 다면체가 모두 박스 일 경우 검사해야하는 A의 면은 3 B의 면은 3 선분도 3 개씩 3 + 3  + (3*3) 15개의 축에 대해서 투영해서 검사한다.

	std::vector<Vector3> CheckNormals;
	CheckNormals.reserve(15u);
	CheckNormals.insert(std::end(CheckNormals), std::begin(RhsOBB->WorldFaceNormals), std::end(RhsOBB->WorldFaceNormals));
	CheckNormals.insert(std::end(CheckNormals), std::begin(WorldFaceNormals), std::end(WorldFaceNormals));
	
	for (auto& CheckNormal : CheckNormals)
	{
		if (false == FMath::IsProjectionIntersectAreaAABB(CheckNormal, WorldCenter, WorldFaceNormals, RhsOBB->WorldCenter, RhsOBB->WorldFaceNormals))
			return false;
	}

	return true;
}

bool Engine::OBB::IsCollisionAABB(Geometric* const Rhs, Vector3& PushDir, float& CrossAreaScale) const&
{
	return false;
}
