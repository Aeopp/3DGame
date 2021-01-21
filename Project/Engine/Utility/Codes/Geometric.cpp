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
	TestCurrentRotation = Rotation;
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
	//const std::array<const float, 3u> CrossAxis
	//{
	//	std::fabsf(Max.x - RhsAABB->Min.x),
	//	std::fabsf(Max.y - Min.y),
	//	std::fabsf(Max.z - Min.z),
	//};
	//
	//std::min_element

	return true;
}

bool Engine::AABB::IsCollisionOBB(Geometric* const Rhs, Vector3& PushDir, float& CrossAreaScale) const&
{
	return false;
}

Engine::OBB::OBB(const Vector3 LocalMin, const Vector3 LocalMax)
	: LocalMin{ LocalMin }, LocalMax{ LocalMax }
{
}

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


Engine::Geometric::Type Engine::OBB::GetType() const&
{
	return Engine::Geometric::Type::OBB;
}

void Engine::OBB::Update(const Vector3 Scale, const Vector3 Rotation, const Vector3 Location)&
{
	const Matrix ToWorld = FMath::WorldMatrix(Scale, Rotation, Location);
	Min = FMath::Mul(LocalMin, ToWorld);
	Max = FMath::Mul(LocalMax, ToWorld);
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

	//if (RhsOBB->Min.x > Max.x || RhsOBB->Max.x < Min.x)
	//	return false;
	//if (RhsOBB->Min.y > Max.y || RhsOBB->Max.y < Min.y)
	//	return false;
	//if (RhsOBB->Min.z > Max.z || RhsOBB->Max.z < Min.z)
	//	return false;

	return true;
}

bool Engine::OBB::IsCollisionAABB(Geometric* const Rhs, Vector3& PushDir, float& CrossAreaScale) const&
{
	return false;
}
