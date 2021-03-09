#include "Frustum.h"
#include "imgui.h"
#include <algorithm>
#include "FMath.hpp"
#include <execution>
#include "UtilityGlobal.h"
#include "ResourceSystem.h"
#include "Vertexs.hpp"
#include "GraphicDevice.h"

void Engine::Frustum::Initialize()&
{
	auto Device = GraphicDevice::Instance->GetDevice();
	auto& ResourceSys = ResourceSystem::Instance;
	VertexBuffer = ResourceSys->Get<IDirect3DVertexBuffer9>(L"VertexBuffer_Frustum");
	IndexBuffer = ResourceSys->Get<IDirect3DIndexBuffer9>(L"IndexBuffer_Frustum");
}

Engine::Frustum::~Frustum() noexcept
{
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
	World = InvProjection * CameraWorld;

	std::transform(std::begin(Points), std::end(Points), std::begin(Points),
		[_World = World](const Vector3& Point)
		{
			return FMath::Mul(Point, _World);
		});

	// Near
	D3DXPlaneFromPoints(&Planes[0], &Points[1], &Points[4], &Points[0]);
	// Far
	D3DXPlaneFromPoints(&Planes[1], &Points[3], &Points[6], &Points[2]);
	// Left
	D3DXPlaneFromPoints(&Planes[2], &Points[0], &Points[4], &Points[3]);
	// Right
	D3DXPlaneFromPoints(&Planes[3], &Points[2], &Points[5], &Points[1]);
	// Top
	D3DXPlaneFromPoints(&Planes[4], &Points[5], &Points[7], &Points[4]);
	// Botton
	D3DXPlaneFromPoints(&Planes[5], &Points[2], &Points[1], &Points[3]);
}

bool Engine::Frustum::IsIn(const Vector3& Point)&
{
	return std::all_of( std::begin(Planes), std::end(Planes),
		[Point](const D3DXPLANE& Plane)
		{
			return D3DXPlaneDotCoord(&Plane, &Point) > 0.f;
		});
}

bool Engine::Frustum::IsIn(const Sphere& _Sphere)&
{
	return std::all_of( std::begin(Planes), std::end(Planes),
		[_Sphere](const D3DXPLANE& Plane)
		{
			const float Distance = D3DXPlaneDotCoord(&Plane, &_Sphere.Center);
			return  (Distance >= _Sphere.Radius) || (std::fabsf(Distance) <= _Sphere.Radius);
		});
}

void Engine::Frustum::Render(IDirect3DDevice9* const Device)&
{
	if (Engine::Global::bDebugMode)
	{
	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	Device->SetRenderState(D3DRS_ZENABLE, FALSE);
	Device->SetTransform(D3DTS_WORLD, &World);
	Device->SetStreamSource(0, VertexBuffer, 0, sizeof(Vector3));
	Device->SetIndices(IndexBuffer);
	Device->SetFVF(D3DFVF_XYZ);
	Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	Device->SetRenderState(D3DRS_ZENABLE, TRUE);

	}
}