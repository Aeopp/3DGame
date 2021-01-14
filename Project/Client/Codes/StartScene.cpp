#include "..\\stdafx.h"
#include "StartScene.h"
#include "Management.h"
#include "HeightMap.h"
#include "EnemyLayer.h"
#include "ExportUtility.hpp"
#include "Controller.h"
#include "App.h"
#include <vector>
#include <array>
#include <numbers>
#include "Layer.h"
#include <iostream>

static constexpr float pi = std::numbers::pi;

struct Location3DUV
{
    Vector3 Location;
    Vector3 UV;
    static inline constexpr uint32_t FVF = D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0);
};

struct _16_t
{
    WORD _1, _2, _3;
    static inline constexpr D3DFORMAT Format = D3DFMT_INDEX16;

};
IDirect3DCubeTexture9* CubeTexture{ nullptr };

float Dot(const Vector3& Lhs, const Vector3& Rhs)
{
    return D3DXVec3Dot(&Lhs, &Rhs);
}
Vector3 Cross(const Vector3& Lhs, const Vector3& Rhs)
{
    Vector3 ReturnValue;
    D3DXVec3Cross(&ReturnValue, &Lhs, &Rhs);
    return ReturnValue;
}

Vector3 Normalize(const Vector3& Lhs)
{
    Vector3 NormVec;
    D3DXVec3Normalize(&NormVec, &Lhs);
    return NormVec;
}
bool InnerPointFromFace(const Vector3& Point, const std::array<Vector3, 3ul>& Face)
{
    std::array <Vector3, 3ul> ToVertexs;

    for (size_t i = 0; i < ToVertexs.size(); ++i)
    {
        ToVertexs[i] = Normalize(Face[i] - Point);
    }

    float Radian = 0;
    Radian += std::acosf(Dot(ToVertexs[0], ToVertexs[1]));
    Radian += std::acosf(Dot(ToVertexs[1], ToVertexs[2]));
    Radian += std::acosf(Dot(ToVertexs[2], ToVertexs[0]));

    return almost_equal(Radian, pi * 2.f);
};

Vector3 GetNormalFromFace(const Vector3& p0, const Vector3& p1, const Vector3& p2)
{
    const Vector3 u = p1 - p0;
    const Vector3 v = p2 - p0;
    return Normalize(Cross(u, v));
}



void StartScene::Initialize(IDirect3DDevice9* const Device)&
{
    Super::Initialize(Device);

	GetProto().LoadPrototype <Engine::HeightMap>(L"Standard");

	GetManager().NewLayer<EnemyLayer>();

	auto _pptr = GetManager().NewObject<EnemyLayer, Engine::HeightMap>(
		L"Standard",L"Name", Engine::RenderInterface::Group::Enviroment);
};

void StartScene::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);


	auto _Map = GetManager().FindObject<EnemyLayer, Engine::HeightMap>(L"Name");
	auto _Layer = GetManager().FindLayer<EnemyLayer>();

	auto& LeyerMap = GetManager().RefLayers();
	auto Objs = GetManager().FindObjects<EnemyLayer, Engine::HeightMap>();
	auto Objs2 =GetManager().RefObjects<EnemyLayer>();


}





