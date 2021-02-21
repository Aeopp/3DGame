#include "Geometric.h"
#include "FMath.hpp"
#include "imgui.h"
#include <iostream>
#include "Vertexs.hpp"
#include <string>
#include "ResourceSystem.h"
#include "imgui.h"
#include <map>

static uint32 CollisionResourceID = 0u;

Engine::OBB::OBB(const Vector3 LocalMin, const Vector3 LocalMax)
	: Geometric{  FMath::Length((LocalMax - LocalMin)) /2.f, (LocalMin + LocalMax) / 2.f  } ,
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
    }, 
	LocalFaceNormals 
	{
	    // +X
		FMath::GetNormalFromFace(LocalPoints[1],LocalPoints[5],LocalPoints[6]) ,
		// +Y
		FMath::GetNormalFromFace(LocalPoints[4],LocalPoints[5],LocalPoints[1]),
		// +Z
		FMath::GetNormalFromFace(LocalPoints[7],LocalPoints[6],LocalPoints[5])
	} ,
	LocalHalfDistances
	{
		(LocalMax.x -LocalMin.x)/2.f ,
		(LocalMax.y -LocalMin.y)/2.f ,
		(LocalMax.z -LocalMin.z)/2.f ,
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
		(L"VertexBuffer_Location3DUV_Cube_Collision_" + std::to_wstring(++CollisionResourceID), VertexBuffer);

	IndexBuffer = ResourceSys->Get<IDirect3DIndexBuffer9>(L"IndexBuffer_Cube");
	CollisionTexture = ResourceSys->Get<IDirect3DTexture9>(L"Texture_Red");
	NoCollisionTexture = ResourceSys->Get<IDirect3DTexture9>(L"Texture_Green");
}


Engine::Geometric::Type Engine::OBB::GetType() const&
{
	return Engine::Geometric::Type::OBB;
};

void Engine::OBB::Update(const Vector3 Scale, const Vector3 Rotation, const Vector3 Location,
	const Matrix& OffsetMatrix, const Vector3& OffsetScale)&
{
	const Matrix ToWorld =  OffsetMatrix*FMath::WorldMatrix(Scale, Rotation, Location);
	WorldCenter = FMath::Mul(LocalCenter, ToWorld);
	std::transform(std::begin(LocalFaceNormals), std::end(LocalFaceNormals), std::begin(WorldFaceNormals),
		[ToWorld](const Vector3& LocalFaceNormal) {
			return  FMath::Normalize(FMath::MulNormal(LocalFaceNormal, ToWorld));
		});
	std::transform(std::begin(LocalPoints), std::end(LocalPoints), std::begin(WorldPoints),
		[ToWorld](const Vector3& LocalPoint) {
			return FMath::Mul(LocalPoint, ToWorld);
		});
	const Vector3 FinalScale = { OffsetScale.x * Scale.x , OffsetScale.y * Scale.y ,OffsetScale.z * Scale.z };

	WorldSphere.Center = FMath::Mul(LocalSphere.Center, ToWorld);
	WorldSphere.Radius = LocalSphere.Radius * FMath::MaxScala(FinalScale);
	WorldHalfDistances = { LocalHalfDistances.x * FinalScale.x  ,
						   LocalHalfDistances.y * FinalScale.y  ,
						   LocalHalfDistances.z * FinalScale.z };
}

void Engine::OBB::Update(const Matrix& ToOwnerWorld, const Vector3& OwnerScale , const Matrix& OffsetMatrix ,const Vector3& OffsetScale )&
{
	WorldCenter = FMath::Mul(LocalCenter, ToOwnerWorld);
	std::transform(std::begin(LocalFaceNormals), std::end(LocalFaceNormals), std::begin(WorldFaceNormals),
		[ToOwnerWorld](const Vector3& LocalFaceNormal) {
			return  FMath::Normalize(FMath::MulNormal(LocalFaceNormal, ToOwnerWorld));
		});
	std::transform(std::begin(LocalPoints), std::end(LocalPoints), std::begin(WorldPoints),
		[ToOwnerWorld](const Vector3& LocalPoint) {
			return FMath::Mul(LocalPoint, ToOwnerWorld);
		});
	const Vector3 FinalScale = { OffsetScale.x * OwnerScale.x , OffsetScale.y * OwnerScale.y ,OffsetScale.z * OwnerScale.z };

	WorldSphere.Center = FMath::Mul(LocalSphere.Center, ToOwnerWorld);
	WorldSphere.Radius = LocalSphere.Radius * FMath::MaxScala(FinalScale);
	WorldHalfDistances = { LocalHalfDistances.x * FinalScale.x  ,
						   LocalHalfDistances.y * FinalScale.y  ,
						   LocalHalfDistances.z * FinalScale.z };
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

std::optional<std::pair<float, Vector3>> Engine::OBB::IsCollision(Geometric* const Rhs)&
{
	if (FMath::Length(WorldSphere.Center - Rhs->WorldSphere.Center) > (WorldSphere.Radius + Rhs->WorldSphere.Radius))
		return {};

	switch (Rhs->GetType())
	{
	case Type::OBB:
		return IsCollisionOBB(static_cast<Engine::OBB*const>(Rhs));
		break;
	case Type::Sphere:
		return IsCollisionSphere(static_cast<Engine::GSphere* const>(Rhs));
		break;
	default:
		break;
	}

	return {};
}

std::optional<std::pair<float, Vector3>> Engine::OBB::IsCollisionSphere(GSphere* const Rhs) const&
{
	return FMath::IsSphereToOBB(Rhs->WorldSphere.Center,
		Rhs->WorldSphere.Radius,
		WorldCenter,
		WorldFaceNormals,
		WorldHalfDistances);
}

std::optional<std::pair<float,Vector3>> Engine::OBB::IsCollisionOBB(OBB* const Rhs) const&
{
	std::vector<Vector3> CheckNormals;
	CheckNormals.reserve(15u);
	CheckNormals.insert(std::end(CheckNormals), std::begin(WorldFaceNormals), std::end(WorldFaceNormals));
	std::copy_if(std::begin(Rhs->WorldFaceNormals), std::end(Rhs->WorldFaceNormals), std::back_inserter(CheckNormals),
		[&LhsWorldFaceNormals = WorldFaceNormals](const Vector3& Target)
	{
		for (auto& RhsNormal : LhsWorldFaceNormals)
		{
			if (FMath::Equal(RhsNormal, Target))return false;
		}
		return true;
	});

	
	// 선택사항 1.
	// Edge 와 Edge 가 충돌하는 경우 두 Edge가 이루는 평면의 수직(normal)이 분리축이다.
	// 때문에 Edge 충돌까지 검출한다면 Edge의 개수 * Edge의 개수 만큼 외적하여서 만든 축으로 검사를 수행해야 한다.
	// 그러나 Edge VS Edge 충돌은 많이 발생하는 조합이 아니며 충돌 깊이가 아주 조금이라도 깊어지면 면과 면의 충돌에서 검출이 된다.
	
	//for (auto& LhsNormal : WorldFaceNormals)
	//{
	//	for (auto& RhsNormal : Rhs->WorldFaceNormals)
	//	{
	//		// 외적해서 새로운 SA후보를 선정할때 두 벡터가 같을경우 외적은 성립하지 않으므로 제외한다.
	//		if (FMath::Equal(LhsNormal, RhsNormal))continue;

	//			const Vector3 CrossVec = (FMath::Cross(LhsNormal, RhsNormal));
	//			CheckNormals.push_back((FMath::Normalize ( CrossVec) ));
	//	}
	//}

	std::map<float, Vector3> ProjectionAreaMap;

	for (auto& CheckNormal : CheckNormals)
	{
		auto ProjectionArea = FMath::IsProjectionIntersectAreaAABB(CheckNormal, WorldCenter, WorldPoints, Rhs->WorldCenter, Rhs->WorldPoints);

		if (false == ProjectionArea.has_value())
			return {};

		const auto [LhsMin, LhsMax, RhsMin, RhsMax]  = *ProjectionArea;

		ProjectionAreaMap[std::fabsf(LhsMax-RhsMin)] = CheckNormal;
        ProjectionAreaMap[std::fabsf(RhsMax-LhsMin)] = -CheckNormal;
	}

	return { *ProjectionAreaMap.begin()};
}


Engine::GSphere::GSphere(const float Radius, const Vector3 Center)
	:Geometric(Radius,Center) 
{

}

void Engine::GSphere::MakeDebugCollisionSphere(IDirect3DDevice9* const Device)
{
	auto& ResourceSys = ResourceSystem::Instance;

	ID3DXBuffer* _SphereAdjacency{ nullptr };
	D3DXCreateSphere(Device, LocalSphere.Radius, 16u, 16u, &_SphereMesh, &_SphereAdjacency);

	uint8* VertexBufferPtr{ nullptr };
	_SphereMesh->LockVertexBuffer(0,reinterpret_cast<void**> (&VertexBufferPtr ) );
	const uint32 Stride = _SphereMesh->GetNumBytesPerVertex();
	for (uint32 Idx = 0u; Idx < _SphereMesh->GetNumVertices(); ++Idx)
	{
		Vector3* const VertexLocationPtr =
			reinterpret_cast<Vector3* const> ((VertexBufferPtr + (Idx * Stride)));

		*VertexLocationPtr += LocalSphere.Center;
	}
	_SphereMesh->UnlockVertexBuffer();

	
	ResourceSys->Insert<ID3DXMesh>
		(L"Mesh_Sphere_Collision_"+      std::to_wstring(++CollisionResourceID), _SphereMesh);
	ResourceSys->Insert<ID3DXBuffer>
		(L"Adjacency_Sphere_Collision_"+ std::to_wstring(++CollisionResourceID), _SphereAdjacency);

	CollisionTexture = ResourceSys->Get<IDirect3DTexture9>(L"Texture_Red");
	NoCollisionTexture = ResourceSys->Get<IDirect3DTexture9>(L"Texture_Green");
}

Engine::Geometric::Type Engine::GSphere::GetType() const&
{
	return Engine::Geometric::Type::Sphere;
}

void Engine::GSphere::Update(const Vector3 Scale, const Vector3 Rotation, const Vector3 Location,
							const Matrix& OffsetMatrix , const Vector3& OffsetScale )&
{
	const Matrix ToWorld = OffsetMatrix * FMath::WorldMatrix(Scale, Rotation, Location);
	WorldSphere.Center = FMath::Mul(LocalSphere.Center, ToWorld);
	const Vector3 FinalScale = { OffsetScale.x * Scale.x , OffsetScale.y * Scale.y ,OffsetScale.z * Scale.z };
	WorldSphere.Radius = LocalSphere.Radius * FMath::MaxScala(FinalScale);
}

void Engine::GSphere::Update(const Matrix& ToOwnerWorld, const Vector3& OwnerScale, const Matrix& OffsetMatrix , const Vector3& OffsetScale)&
{
	const Vector3 FinalScale = { OffsetScale.x * OwnerScale.x , OffsetScale.y * OwnerScale.y ,OffsetScale.z * OwnerScale.z };
	WorldSphere.Center = FMath::Mul(LocalSphere.Center, ToOwnerWorld);
	WorldSphere.Radius = LocalSphere.Radius * FMath::MaxScala(FinalScale);
}

void Engine::GSphere::Render(IDirect3DDevice9* const Device, const bool bCurrentUpdateCollision)&
{
	IDirect3DTexture9* DebugTexture = bCurrentUpdateCollision ? CollisionTexture : NoCollisionTexture;
	Device->SetTexture(0, DebugTexture);
	_SphereMesh->DrawSubset(0);
}

std::optional<std::pair<float, Vector3>> Engine::GSphere::IsCollision(Geometric* const Rhs)&
{
	switch (Rhs->GetType())
	{
	case Type::OBB:
		if (FMath::Length(WorldSphere.Center - Rhs->WorldSphere.Center) > (WorldSphere.Radius + Rhs->WorldSphere.Radius))
		{
			return {};
		}
		else
		{
			return IsCollisionOBB(static_cast<Engine::OBB* const>(Rhs));
		}
		break;
	case Type::Sphere:
		return IsCollisionSphere(static_cast<Engine::GSphere* const>(Rhs));
		break;
	default:
		break;
	}

	return {};
}

std::optional<std::pair<float, Vector3>> Engine::GSphere::IsCollisionOBB(
	OBB* const Rhs) const&
{
	return FMath::IsSphereToOBB(
		WorldSphere.Center, WorldSphere.Radius,
		Rhs->WorldCenter, Rhs->WorldFaceNormals,
		Rhs->WorldHalfDistances);
};

std::optional<std::pair<float, Vector3>> Engine::GSphere::IsCollisionSphere(GSphere* const Rhs) const&
{
	const Vector3 ToRhs = Rhs->WorldSphere.Center - WorldSphere.Center;
	const float Distance = FMath::Length(ToRhs);
	const float RadiusSum = WorldSphere.Radius + Rhs->WorldSphere.Radius;
	if (Distance > RadiusSum)return {};
	else
	{
		return {{ RadiusSum - Distance , FMath::Normalize(ToRhs) }};
	}
}
