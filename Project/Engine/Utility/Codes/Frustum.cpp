#include "Frustum.h"
#include <algorithm>
#include "FMath.hpp"
#include <execution>
#include "ResourceSystem.h"

void Engine::Frustum::Initialize()&
{
	auto& ResourceSys = ResourceSystem::Instance;
	//ResourceSys->Emplace(L"")
}

void Engine::Frustum::Make(const Matrix& CameraWorld, const Matrix& Projection)&
{
	// �ð� ���� �ǰ��� ������ ���� �ϴܺ��� �����Ͽ�����
	// Near ��� 0->4->5->1
	// Far ��� 3->7->6->2
	Points[0].x = -1.f, Points[0].y = -1.f, Points[0].z = 0.f;
	Points[1].x = +1.f, Points[1].y = -1.f, Points[1].z = 0.f;
	Points[2].x = +1.f, Points[2].y = -1.f, Points[2].z = 1.f;
	Points[3].x = -1.f, Points[3].y = -1.f, Points[3].z = 1.f;

	Points[4].x = -1.f, Points[4].y = +1.f, Points[4].z = 0.f;
	Points[5].x = +1.f, Points[5].y = +1.f, Points[5].z = 0.f;
	Points[6].x = +1.f, Points[6].y = +1.f, Points[6].z = 1.f;
	Points[7].x = -1.f, Points[7].y = +1.f, Points[7].z = 1.f;

	// ������������� p�� �ϰ� ����ü�� f�� �Ͽ�����.
	// ������ �Ҷ� View ��ǥ�ý��ۿ��� f �� ���ο� ������ ��������
	// -1 ~ +1 (xy) 0 ~ +1 (z) �� ����ȭ�� ��ǥ�ý���(NDC)���� �ٲܼ� �ִ�.

	// �ݴ�� NDC ��ǥ������ ��� ������ �����ϴ�
	// Box�� �𵨸� �� ���� p^-1 �� ���ϸ�
	// p�� �����Ҷ� �𵨸��� f�� ���� �� �ִ�.
	// f ���� �������� NDC�� (p�� ���ؼ�) �ٲپ�����
	// NDC�� �𵨸��� Box�� p^-1 �� ���ϸ� f�� ���� �� �ִ� ��. 
	// ���⼭ ���� f�� f�� ������ǥ���̹Ƿ� ī�޶��� ��������� ���ؼ�
	// f�� ī�޶� ��ġ�� �̵����Ѽ� ������ǥ������ f�� �����.
	const Matrix InvProjection = FMath::Inverse(Projection);
	const Matrix ToWorld = InvProjection * CameraWorld;

	std::transform(std::execution::par,
		std::begin(Points), std::end(Points), std::begin(Points),
		[ToWorld](const Vector3& Point)
		{
			return FMath::Mul(Point, ToWorld);
		});
	CameraLocation = { CameraWorld._41,CameraWorld._42,CameraWorld._43 };

	// Near
	D3DXPlaneFromPoints(&Planes[0], &Points[1], &Points[4], &Points[5]);
	// Far
	D3DXPlaneFromPoints(&Planes[1], &Points[3], &Points[6], &Points[2]);
	// Left
	D3DXPlaneFromPoints(&Planes[2], &Points[0], &Points[4], &Points[3]);
	// Right
	D3DXPlaneFromPoints(&Planes[3], &Points[2], &Points[5], &Points[1]);
	// Top
	D3DXPlaneFromPoints(&Planes[4], &Points[5], &Points[7], &Points[4]);
	// Botton
	D3DXPlaneFromPoints(&Planes[5], &Points[2], &Points[0], &Points[3]);

	std::for_each(std::execution::par_unseq,
		std::begin(Planes), std::end(Planes),
		[ToWorld](D3DXPLANE& Plane)
		{
			Plane.d *= -1.f;
		});
}

bool Engine::Frustum::IsIn(const Vector3& Point)&
{
	return std::all_of(std::execution::seq, std::begin(Planes), std::end(Planes),
		[Point](const D3DXPLANE& Plane)
		{
			return D3DXPlaneDotCoord(&Plane, &Point) > 0.f;
		});
}

bool Engine::Frustum::IsIn(const Sphere& _Sphere)&
{
	return std::all_of(std::execution::seq, std::begin(Planes), std::end(Planes),
		[_Sphere](const D3DXPLANE& Plane)
		{
			const float Distance = D3DXPlaneDotCoord(&Plane, &_Sphere.Center);
			return (Distance > 0.f) || (std::fabsf(Distance) < _Sphere.Radius);
		});
}

void Engine::Frustum::Render(IDirect3DDevice9* const Device)&
{
	
	//WORD		index[] = { 0, 5, 1,
	//						0, 4, 5,
	//						3, 4, 0,
	//						0, 7, 4,
	//						1, 6, 2,
	//						1, 5, 6,
	//						4, 6, 5,
	//						4, 7, 6,
	//						0, 1, 2,
	//						0, 3, 2,
	//						6, 7, 3,
	//						6, 3, 2 };

	//D3DMATERIAL9 mtrl;
	//ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));

	//typedef struct tagVTX
	//{
	//	D3DXVECTOR3	p;
	//} VTX;

	//VTX		vtx[8];

	//for (int i = 0; i < 8; i++)
	//	vtx[i].p = Points[i];

	//Matrix world;
	//world = FMath::Identity(world); 
	//Device->SetTransform(D3DTS_WORLD, & world );  
	//Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	//Device->SetFVF(D3DFVF_XYZ);
	//Device->SetStreamSource(0, NULL, 0, sizeof(VTX));
	//Device->SetTexture(0, NULL);
	//Device->SetIndices(0);
	//Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	//Device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	//Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	//Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	//Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	//// �Ķ������� ��,�� ����� �׸���.

	//Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	//ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
	//mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
	//Device->SetMaterial(&mtrl);
	//Device->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 8, 12, index, D3DFMT_INDEX16, vtx, sizeof(vtx[0]));

	//// ������� ��,�� ����� �׸���.
	//ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
	//mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	//Device->SetMaterial(&mtrl);
	//Device->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 8, 4, index + 4 * 3, D3DFMT_INDEX16, vtx, sizeof(vtx[0]));

	//// ���������� ��,�� ����� �׸���.
	//ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
	//mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	//Device->SetMaterial(&mtrl);
	//Device->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 8, 4, index + 8 * 3, D3DFMT_INDEX16, vtx, sizeof(vtx[0]));

	//Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	//Device->SetRenderState(D3DRS_LIGHTING, FALSE);
}
