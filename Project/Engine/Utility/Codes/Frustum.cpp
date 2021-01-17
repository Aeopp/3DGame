#include "Frustum.h"
#include "imgui.h"
#include <algorithm>
#include "FMath.hpp"
#include <execution>
#include "ResourceSystem.h"
#include "Vertexs.hpp"
#include "GraphicDevice.h"


IDirect3DVertexBuffer9* VB = 0;
IDirect3DIndexBuffer9* IB = 0;

struct VRETEX
{
	VRETEX() {}
	VRETEX(float x, float y, float z)
	{
		_x = x;  _y = y;  _z = z;
	}
	float _x, _y, _z;
	static const DWORD FVF;
};
const DWORD VRETEX::FVF = D3DFVF_XYZ;

void Engine::Frustum::Initialize()&
{
	auto Device = GraphicDevice::Instance->GetDevice();

	Device->CreateVertexBuffer(
		8 * sizeof(VRETEX),
		D3DUSAGE_WRITEONLY,
		VRETEX::FVF,
		D3DPOOL_MANAGED,
		&VB,
		0);

	Device->CreateIndexBuffer(
		36 * sizeof(WORD),
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&IB,
		0);

	//
	// Fill the buffers with the cube data.
	//

	// define unique vertices:
	VRETEX* vertices;
	VB->Lock(0, 0, (void**)&vertices, 0);

	// vertices of a unit cube
	vertices[0] = VRETEX(-1.0f, -1.0f, 0.f);
	vertices[1] = VRETEX(-1.0f, 1.0f, 0.f);
	vertices[2] = VRETEX(1.0f, 1.0f, 0.f);
	vertices[3] = VRETEX(1.0f, -1.0f, 0.f);
	vertices[4] = VRETEX(-1.0f, -1.0f, 1.0f);
	vertices[5] = VRETEX(-1.0f, 1.0f, 1.0f);
	vertices[6] = VRETEX(1.0f, 1.0f, 1.0f);
	vertices[7] = VRETEX(1.0f, -1.0f, 1.0f);

	VB->Unlock();

	// define the triangles of the cube:
	WORD* indices = 0;
	IB->Lock(0, 0, (void**)&indices, 0);

	// front side
	indices[0] = 0; indices[1] = 1; indices[2] = 2;
	indices[3] = 0; indices[4] = 2; indices[5] = 3;

	// back side
	indices[6] = 4; indices[7] = 6; indices[8] = 5;
	indices[9] = 4; indices[10] = 7; indices[11] = 6;

	// left side
	indices[12] = 4; indices[13] = 5; indices[14] = 1;
	indices[15] = 4; indices[16] = 1; indices[17] = 0;

	// right side
	indices[18] = 3; indices[19] = 2; indices[20] = 6;
	indices[21] = 3; indices[22] = 6; indices[23] = 7;

	// top
	indices[24] = 1; indices[25] = 5; indices[26] = 6;
	indices[27] = 1; indices[28] = 6; indices[29] = 2;

	// bottom
	indices[30] = 4; indices[31] = 0; indices[32] = 3;
	indices[33] = 4; indices[34] = 3; indices[35] = 7;

	IB->Unlock();

}

void Engine::Frustum::Make(const Matrix& CameraWorld, const Matrix& Projection)&
{
	// 시계 방향 되감기 순서로 좌측 하단부터 시작하였을때
	// Near 평면 0->4->5->1
	// Far 평면 3->7->6->2
	Points[0].x = -1.f, Points[0].y = -1.f, Points[0].z = 0.f;
	Points[1].x = +1.f, Points[1].y = -1.f, Points[1].z = 0.f;
	Points[2].x = +1.f, Points[2].y = -1.f, Points[2].z = 1.f;
	Points[3].x = -1.f, Points[3].y = -1.f, Points[3].z = 1.f;

	Points[4].x = -1.f, Points[4].y = +1.f, Points[4].z = 0.f;
	Points[5].x = +1.f, Points[5].y = +1.f, Points[5].z = 0.f;
	Points[6].x = +1.f, Points[6].y = +1.f, Points[6].z = 1.f;
	Points[7].x = -1.f, Points[7].y = +1.f, Points[7].z = 1.f;

	// 원근투영행렬을 p라 하고 절두체를 f라 하였을때.
	// 렌더링 할때 View 좌표시스템에서 f 의 내부에 존재한 정점들을
	// -1 ~ +1 (xy) 0 ~ +1 (z) 로 정규화한 좌표시스템(NDC)으로 바꿀수 있다.

	// 반대로 NDC 좌표에서의 모든 정점을 포함하는
	// Box를 모델링 한 다음 p^-1 를 곱하면
	// p를 정의할때 모델링한 f를 얻을 수 있다.
	// f 안의 정점들을 NDC로 (p를 곱해서) 바꾸었으니
	// NDC를 모델링한 Box에 p^-1 를 곱하면 f를 얻을 수 있는 것. 
	// 여기서 얻은 f는 f의 로컬좌표상이므로 카메라의 월드행렬을 곱해서
	// f를 카메라 위치로 이동시켜서 월드좌표에서의 f로 만든다.
	const Matrix InvProjection = FMath::Inverse(Projection);
	World = InvProjection * CameraWorld;
	 
	std::transform(std::execution::par,
		std::begin(Points), std::end(Points), std::begin(Points),
		[_World=World](const Vector3& Point)
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
	ImGui::Begin("Frustum");
	for (size_t i = 0u; i < Planes.size(); ++i)
	{
		const auto& Plane = Planes[i];
		std::wstring Name;
		switch (i)
		{
		case 0:
			Name = L"Near";
			break;
		case 1:
			Name = L"Far";
			break;
		case 2:
			Name = L"Left";
			break;
		case 3:
			Name = L"Right";
			break;
		case 4:
			Name = L"Top";
			break;
		case 5:
			Name = L"Bottom";
			break;
		default:
			break;
		}
		ImGui::Text("Distance From %s : %f\n", Name.c_str(),D3DXPlaneDotCoord(&Plane, &Point));
	}
	ImGui::End();

	return std::all_of(std::execution::seq, std::begin(Planes), std::end(Planes),
		[Point](const D3DXPLANE& Plane)
		{
			return D3DXPlaneDotCoord(&Plane, &Point) > 0.f;
		});
}

bool Engine::Frustum::IsIn(const Sphere& _Sphere)&
{
	ImGui::Begin("Frustum");
	for (size_t i = 0u; i < Planes.size(); ++i)
	{
		const auto& Plane = Planes[i];
		std::wstring Name;
		switch (i)
		{
		case 0:
			Name = L"Near";
			break;
		case 1:
			Name = L"Far";
			break;
		case 2:
			Name = L"Left";
			break;
		case 3:
			Name = L"Right";
			break;
		case 4:
			Name = L"Top";
			break;
		case 5:
			Name = L"Bottom";
			break;
		default:
			break;
		}
		ImGui::Text("Distance From %s : %f\n", Name.c_str(), D3DXPlaneDotCoord(&Plane, &_Sphere.Center));
	}
	ImGui::End();

	return std::all_of(std::execution::seq, std::begin(Planes), std::end(Planes),
		[_Sphere](const D3DXPLANE& Plane)
		{
			const float Distance = D3DXPlaneDotCoord(&Plane, &_Sphere.Center);
			return (Distance > 0.f) || (std::fabsf(Distance) < _Sphere.Radius);
		});
}

void Engine::Frustum::Render(IDirect3DDevice9* const Device)&
{
	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	Device->SetRenderState(D3DRS_ZENABLE, FALSE);
	Device->SetTransform(D3DTS_WORLD, &World);
	Device->SetStreamSource(0, VB, 0, sizeof(VRETEX));
	Device->SetIndices(IB);
	Device->SetFVF(VRETEX::FVF);
	Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	Device->SetRenderState(D3DRS_ZENABLE, TRUE);
}
