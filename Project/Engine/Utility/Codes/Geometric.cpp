#include "Geometric.h"
#include "FMath.hpp"
#include "imgui.h"
#include <iostream>
#include "Vertexs.hpp"
#include <string>
#include "ResourceSystem.h"
#include "imgui.h"



static uint32 DebugVertexBufferID = 0u;

Engine::AABB::AABB(const Vector3 LocalMin, const Vector3 LocalMax)
	:  
	Geometric{ FMath::Length((LocalMax - LocalMax)) , (LocalMin + LocalMax) / 2.f } ,
	LocalMin{ LocalMin }, LocalMax{ LocalMax }   
{
}

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

	// ����
	VertexBufferPtr[0].Location = Vector3(LocalMin.x, LocalMax.y, LocalMin.z);
	VertexBufferPtr[0].UV = VertexBufferPtr[0].Location;

	VertexBufferPtr[1].Location = Vector3(LocalMax.x, LocalMax.y, LocalMin.z);
	VertexBufferPtr[1].UV = VertexBufferPtr[1].Location;

	VertexBufferPtr[2].Location = Vector3(LocalMax.x, LocalMin.y, LocalMin.z);
	VertexBufferPtr[2].UV = VertexBufferPtr[2].Location;

	VertexBufferPtr[3].Location = Vector3(LocalMin.x, LocalMin.y, LocalMin.z);
	VertexBufferPtr[3].UV = VertexBufferPtr[3].Location;

	// �ĸ�
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
	WorldSphere.Center = FMath::Mul(LocalSphere.Center, ToWorld);
	WorldSphere.Radius = LocalSphere.Radius *   ((Scale.x + Scale.y + Scale.z) /3.f);   
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
	: Geometric{  FMath::Length((LocalMax - LocalMax)) , (LocalMin + LocalMax) / 2.f  } ,
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
	    // +X
		FMath::GetNormalFromFace(LocalPoints[1],LocalPoints[5],LocalPoints[6]) ,
		// +Y
		FMath::GetNormalFromFace(LocalPoints[4],LocalPoints[5],LocalPoints[1]),
		// +Z
		FMath::GetNormalFromFace(LocalPoints[7],LocalPoints[6],LocalPoints[5])
	} 
	{};

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
	WorldSphere.Center = FMath::Mul(LocalSphere.Center, ToWorld);
	WorldSphere.Radius = LocalSphere.Radius * ((Scale.x + Scale.y + Scale.z) / 3.f);
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
	if (FMath::Length(WorldSphere.Center - Rhs->WorldSphere.Center) > (WorldSphere.Radius + Rhs->WorldSphere.Radius))
		return false;

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
	auto RhsOBB = static_cast<OBB*const>(Rhs);

	std::vector<Vector3> CheckNormals;
	CheckNormals.reserve(15u);
	CheckNormals.insert(std::end(CheckNormals), std::begin(RhsOBB->WorldFaceNormals), std::end(RhsOBB->WorldFaceNormals));
	CheckNormals.insert(std::end(CheckNormals), std::begin(WorldFaceNormals), std::end(WorldFaceNormals));

	for (auto& LhsNormal : WorldFaceNormals)
	{
		for (auto& RhsNormal : RhsOBB->WorldFaceNormals)
		{
			CheckNormals.push_back(FMath::Cross(LhsNormal, RhsNormal));
		}
	}

	for (auto& CheckNormal : CheckNormals)
	{
		if (false == FMath::IsProjectionIntersectAreaAABB(CheckNormal, WorldCenter, WorldPoints, RhsOBB->WorldCenter, RhsOBB->WorldPoints))
			return false;
	}

	return true;
}

bool Engine::OBB::IsCollisionAABB(Geometric* const Rhs, Vector3& PushDir, float& CrossAreaScale) const&
{
	return false;
}
