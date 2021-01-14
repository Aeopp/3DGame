#include "..\\stdafx.h"
#include "StartScene.h"
#include "Management.h"
#include "EnemyLayer.h"
#include "HeightMap.h"
#include "ExportUtility.hpp"
#include "Controller.h"
#include "App.h"
#include <vector>
#include <array>
#include <numbers>

static constexpr float pi = std::numbers::pi;

IDirect3DVertexBuffer9* CubeMapVertexBuffer;
IDirect3DIndexBuffer9* CubeMapIndexBuffer = nullptr;
IDirect3DCubeTexture9* PlayerTexture;
IDirect3DVertexBuffer9* PlayerCubeVertexBuffer;
IDirect3DIndexBuffer9* PlayerIndexBuffer = nullptr;

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

struct Triangle
{
    Triangle(Vector3 _1, Vector3 _2, Vector3 _3) :
        _1{ _1 }, _2{ _2 }, _3{ _3 }
    {
        Normal = GetNormalFromFace(_1, _2, _3);
    };
    Vector3 _1, _2, _3;
    Vector3 Normal;
};

LPDIRECT3DTEXTURE9  g_pTexDiffuse = nullptr; /// Texture 색깔맵
LPDIRECT3DTEXTURE9      g_pTexHeightMap = nullptr;  // HeightMap에 사용할 텍스처
UINT g_cxHeight;
IDirect3DVertexBuffer9* g_pVB = nullptr;
IDirect3DDevice9* g_pd3dDevice;
LPDIRECT3DINDEXBUFFER9 g_pIB = nullptr; /// 인덱스를 보관할 인덱스버퍼
std::vector<Triangle> MapTriangles{};

    /// 뷰 행렬을 설정

UINT g_czHeight;
struct CUSTOMVERTEX
{
    Vector3 Location;
    static inline const DWORD FVF = D3DFVF_XYZ;
};
struct MYINDEX {
    WORD _0, _1, _2;  /// WORD, 16비트 인덱스
};
/**-----------------------------------------------------------------------------
* 텍스처 초기화
*------------------------------------------------------------------------------
*/



HRESULT InitTexture()
{
    g_pd3dDevice = GetGraphic().GetDevice().get();

    // 높이맵
    if (FAILED(D3DXCreateTextureFromFileEx(g_pd3dDevice,
        L"..\\..\\Resource\\Texture\\Terrain\\Height2.bmp", D3DX_DEFAULT, D3DX_DEFAULT,
        D3DX_DEFAULT, 0,
        D3DFMT_X8R8G8B8, D3DPOOL_MANAGED,
        D3DX_DEFAULT, D3DX_DEFAULT, 0,
        NULL, NULL, &g_pTexHeightMap
    ))) {
        MessageBox(NULL, L"Can't not open the texture file.", L"Error", MB_OK);
        return E_FAIL;
    }


    // D3D에서 텍스처에 대한 정보를 저장하기 위한 구조체
    D3DSURFACE_DESC ddsd;
    // 텍스처 메모리의 포인터를 저장하기 위한 구조체
    D3DLOCKED_RECT d3drc;

    // 텍스처 정보를 읽어온다.
    g_pTexHeightMap->GetLevelDesc(0, &ddsd);
    // 텍스처의 가로, 세로 크기를 저장한다.
    g_cxHeight = ddsd.Width;
    g_czHeight = ddsd.Height;

    // 텍스처의 크기에 맞게 정점 버퍼를 생성한다.
    if (FAILED(g_pd3dDevice->CreateVertexBuffer(g_cxHeight * g_czHeight * 
        sizeof(CUSTOMVERTEX),
        0, CUSTOMVERTEX::FVF, D3DPOOL_DEFAULT, &g_pVB, NULL))) {
        return E_FAIL;
    }

    if (FAILED(g_pTexHeightMap->LockRect(0, &d3drc, NULL, D3DLOCK_READONLY)))
    {
        return E_FAIL;
    }
    VOID* pVertexBuffer = nullptr;
    if (FAILED(g_pVB->Lock(0, g_cxHeight * g_czHeight * sizeof(CUSTOMVERTEX), &pVertexBuffer, 0))) {
        return E_FAIL;
    }

    // 정점의 좌표값을 할당하는 간단한 알고리즘
    // (0, 0, 0)의 위치를 지형의 무게중심으로 하기 위한 작업
    CUSTOMVERTEX v;
    CUSTOMVERTEX* pV = (CUSTOMVERTEX*)pVertexBuffer;

    for (DWORD z = 0; z < g_cxHeight; ++z) {
        for (DWORD x = 0; x < g_czHeight; ++x) {
            // 정점의 좌표값을 설정한다.
            v.Location.x = (float)x - g_cxHeight / 2.f;
            // 텍스처의 색상 및 명암값이 저장된 메모리 주소에 접근하여,
            // 텍스처의 색상 및 명암값과 0x000000ff와 & 연산하면
            // 명암값을 얻어 올 수 있다.
            // 이 값은 0 ~ 255 사이이다.
            // 이 코드에서는 이 값을 10으로 나누어서 높이값으로 설정한다.
            v.Location.y = ((float)(*((LPDWORD)d3drc.pBits + x + z * (d3drc.Pitch / 4)) & 0x000000ff)) / 10.f;
            v.Location.z = -((float)z - g_czHeight / 2.f);

            *pV++ = v;
        }
    }

    // Lock() - Unlock()

    g_pTexHeightMap->UnlockRect(0);

    // 인덱스 버퍼를 생성한다.
  // (가로 * 세로) 갯수만큼의 사각형이 필요하다.
  // 각각의 사각형은 2개의 삼각형으로 이루어진다.
  // 그래서 인덱스 버퍼의 크기는 ( g_cxHeight - 1 )*( g_czHeight - 1 ) * 2 * sizeof( MYINDEX )
    if (FAILED(g_pd3dDevice->CreateIndexBuffer((g_cxHeight - 1) * (g_czHeight - 1) * 2 * sizeof(MYINDEX),
        0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pIB, NULL))) {
        return E_FAIL;
    }

    // 인덱스 버퍼에 저정할 인덱스를 생성하는 알고리즘
    MYINDEX  i;
    MYINDEX* pI;
    if (FAILED(g_pIB->Lock(0, (g_cxHeight - 1) * (g_czHeight - 1) * 2 * sizeof(MYINDEX), (void**)&pI, 0)))
        return E_FAIL;
    pV = (CUSTOMVERTEX*)pVertexBuffer;
    for (DWORD z = 0; z < g_czHeight - 1; z++) {
        for (DWORD x = 0; x < g_cxHeight - 1; x++) {

            // 정점 인덱스를 CW 방향으로 설정하고 있다.
            // 만약 CW 방향으로 컬링 모드를 설정하면 아무것도 그려지지 않을 것이다.
            // g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW ) 인 경우 
            // 그려지는 것이 없다.

            // 좌상 위치의 삼각형
            i._0 = (z * g_cxHeight + x);
            i._1 = (z * g_cxHeight + x + 1);
            i._2 = ((z + 1) * g_cxHeight + x);

            MapTriangles.push_back
            (
                
               Triangle {               pV[i._0].Location,
                                        pV[i._1].Location,
                                         pV[i._2].Location }
            );
            
            // 포인터 연산을 통해 다음 인덱스 저장위치 설정
            *pI++ = i;

            // 우하 위치의 삼각형
            i._0 = ((z + 1) * g_cxHeight + x);
            i._1 = (z * g_cxHeight + x + 1);
            i._2 = ((z + 1) * g_cxHeight + x + 1);

            MapTriangles.push_back
            (
               
                Triangle{             pV[i._0].Location,
                                     pV[i._1].Location,
                                     pV[i._2].Location }
            );

            // 포인터 연산을 통해 다음 인덱스 저장위치 설정
            *pI++ = i;
        }
    }


    g_pVB->Unlock();
    // Lock() - Unlock()
    g_pIB->Unlock();

    if (FAILED(D3DXCreateCubeTextureFromFile(g_pd3dDevice,
        L"..\\..\\Resource\\Texture\\KimCube.dds", &PlayerTexture)))
    {
        throw std::exception(__FUNCTION__);
    }

    if (FAILED(D3DXCreateCubeTextureFromFile(g_pd3dDevice,
        L"..\\..\\Resource\\Texture\\Red.dds", &CubeTexture)))
    {
        throw std::exception(__FUNCTION__);
    }
    

    if (FAILED(g_pd3dDevice->CreateVertexBuffer(
        sizeof(Location3DUV) * 8,
        0, Location3DUV::FVF, D3DPOOL_MANAGED,
        &CubeMapVertexBuffer, nullptr)))
    {
        throw std::exception(__FUNCTION__);
    }

   
    Location3DUV* CubeMapVertexPtr = nullptr;
    CubeMapVertexBuffer->Lock(0, 0, (void**)&CubeMapVertexPtr, 0);

    CubeMapVertexPtr[0].UV = CubeMapVertexPtr[0].Location = Vector3(-0.5f, 0.5f, -0.5f);
    CubeMapVertexPtr[1].UV = CubeMapVertexPtr[1].Location = Vector3(0.5f, 0.5f, -0.5f);
    CubeMapVertexPtr[2].UV = CubeMapVertexPtr[2].Location = Vector3(0.5f, -0.5f, -0.5f);
    CubeMapVertexPtr[3].UV = CubeMapVertexPtr[3].Location = Vector3(-0.5f, -0.5f, -0.5f);
    CubeMapVertexPtr[4].UV = CubeMapVertexPtr[4].Location = Vector3(-0.5f, 0.5f, 0.5f);
    CubeMapVertexPtr[5].UV = CubeMapVertexPtr[5].Location = Vector3(0.5f, 0.5f, 0.5f);
    CubeMapVertexPtr[6].UV = CubeMapVertexPtr[6].Location = Vector3(0.5f, -0.5f, 0.5f);
    CubeMapVertexPtr[7].UV = CubeMapVertexPtr[7].Location = Vector3(-0.5f, -0.5f, 0.5f);

    CubeMapVertexBuffer->Unlock();

    /* 인덱스버퍼 생성 */
    if (FAILED(g_pd3dDevice->CreateIndexBuffer(
        sizeof(_16_t) * 12, /* 인덱스버퍼가 관리할 배열의 총 사이즈 */
        0, /* 0이면 정적버퍼 */
        _16_t::Format, /* 인덱스 포맷 */
        D3DPOOL_MANAGED, /* 메모리 보관 방식 */
        &CubeMapIndexBuffer,
        nullptr)))
    {
        throw std::exception(__FUNCTION__);
    }

    _16_t* _IndexBufPtr = nullptr;
    CubeMapIndexBuffer->Lock(0, 0, (void**)&_IndexBufPtr, 0);

    // +x
    _IndexBufPtr[0]._1 = 1;
    _IndexBufPtr[0]._2 = 5;
    _IndexBufPtr[0]._3 = 6;
    _IndexBufPtr[1]._1 = 1;
    _IndexBufPtr[1]._2 = 6;
    _IndexBufPtr[1]._3 = 2;
    _IndexBufPtr[2]._1 = 4;
    _IndexBufPtr[2]._2 = 0;
    _IndexBufPtr[2]._3 = 3;
    _IndexBufPtr[3]._1 = 4;
    _IndexBufPtr[3]._2 = 3;
    _IndexBufPtr[3]._3 = 7;
    _IndexBufPtr[4]._1 = 4;
    _IndexBufPtr[4]._2 = 5;
    _IndexBufPtr[4]._3 = 1;
    _IndexBufPtr[5]._1 = 4;
    _IndexBufPtr[5]._2 = 1;
    _IndexBufPtr[5]._3 = 0;
    _IndexBufPtr[6]._1 = 3;
    _IndexBufPtr[6]._2 = 2;
    _IndexBufPtr[6]._3 = 6;
    _IndexBufPtr[7]._1 = 3;
    _IndexBufPtr[7]._2 = 6;
    _IndexBufPtr[7]._3 = 7;
    _IndexBufPtr[8]._1 = 5;
    _IndexBufPtr[8]._2 = 4;
    _IndexBufPtr[8]._3 = 7;
    _IndexBufPtr[9]._1 = 5;
    _IndexBufPtr[9]._2 = 7;
    _IndexBufPtr[9]._3 = 6;
    _IndexBufPtr[10]._1 = 0;
    _IndexBufPtr[10]._2 = 1;
    _IndexBufPtr[10]._3 = 2;
    _IndexBufPtr[11]._1 = 0;
    _IndexBufPtr[11]._2 = 2;
    _IndexBufPtr[11]._3 = 3;

    CubeMapIndexBuffer->Unlock();



    return S_OK;
}


void StartScene::Initialize(IDirect3DDevice9* const Device)&
{
	Super::Initialize(Device);
    InitTexture();

	//Engine::Management::Instance->NewObject<EnemyLayer, 
	//	Engine::HeightMap>									(L"HeightMap",Engine::RenderInterface::Group::Enviroment);
}
Vector3 CameraLocation{ 0,0,0 };
Vector3 Forward{ 0,0,1 };
float Speed = 5.f;
Vector3 Up{ 0,1,0 };
Vector3 Right{ 1,0,0 };
void StartScene::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);

    Vector3 MoveForward = Forward;
    MoveForward.y = 0.0f;
    MoveForward   = Normalize(MoveForward);

    Vector3 MoveRight = Right;
    MoveRight.y = 0.0f;
    MoveRight = Normalize(MoveRight);


    if (GetAsyncKeyState('W') & 0x8000)
    {
        
        CameraLocation += MoveForward * Speed * DeltaTime;
    }
    if (GetAsyncKeyState('S') & 0x8000)
    {
        CameraLocation += -MoveForward * Speed * DeltaTime;
    }
    if (GetAsyncKeyState('D') & 0x8000)
    {
        CameraLocation += MoveRight * Speed * DeltaTime;
    }
    if (GetAsyncKeyState('A') & 0x8000)
    {
        CameraLocation += -MoveRight * Speed * DeltaTime;
    }

    auto ClientSize = App::Instance->GetClientSize();
    HWND _Hwnd = App::Instance->GetHwnd();
    POINT Pt;
    GetCursorPos(&Pt);
    ScreenToClient(_Hwnd, &Pt);
    
    float dx = ((float)Pt.x - (ClientSize.first/2) );
    float dy = ((float)Pt.y - (ClientSize.second/2 ) );
    dx = D3DXToRadian(dx)*0.1f;
    dy = D3DXToRadian(dy) * 0.1f;
    Vector3 _Up = { 0,1,0 };
    Matrix rotx, roty;

    D3DXMatrixRotationAxis(&rotx, &_Up, dx);
    D3DXVec3TransformNormal(&Forward, &Forward, &rotx);
    D3DXVec3TransformNormal(&Right, &Right, &rotx);
    D3DXVec3TransformNormal(&Up, &Up, &rotx);

    Forward = Normalize(Forward);
    Right = Normalize(Right);
    Up = Normalize(Up);

    D3DXMatrixRotationAxis(&roty, &Right, dy);
    D3DXVec3TransformNormal(&Forward, &Forward, &roty);
    D3DXVec3TransformNormal(&Up, &Up, &roty);
   D3DXVec3TransformNormal(&Right, &Right, &roty);

    Forward = Normalize(Forward);
    Right = Normalize(Right);
    Up = Normalize(Up);


    Pt.x = ClientSize.first / 2;
    Pt.y = ClientSize.second / 2;
    ClientToScreen(_Hwnd, &Pt);
    SetCursorPos(Pt.x, Pt.y);

    for (const auto& CurrentTriangle : MapTriangles)
    {
        Vector3 Point = CameraLocation;
        Point.y = 0.0f;
        std::array<Vector3, 3u> Face{ CurrentTriangle._1,
                                        CurrentTriangle._2,
                                        CurrentTriangle._3 };
        Face[2].y = Face[1].y = Face[0].y = 0.0f;

        if (InnerPointFromFace(Point, Face))
        {
            const float d = -Dot(CurrentTriangle.Normal, CurrentTriangle._1);
            const float t = (-(
                (Dot(CurrentTriangle.Normal, CameraLocation)) + d))
                /
                (Dot(CurrentTriangle.Normal, Vector3{ 0,-1.f,0.f }));

            //if (t > 0.f)
            {
                CameraLocation = CameraLocation + t * Vector3{ 0,-1.f,0.f };
            }
        
//             const float CameraD = Dot(CurrentTriangle.Normal, CameraLocation);
        }
    }

    Matrix PlayerWorld;
    D3DXMatrixScaling(&PlayerWorld, 0.3f, 0.3f, 0.3f);
    const Vector3 PlayerLocation = CameraLocation;

    PlayerWorld._41 = PlayerLocation.x;
    PlayerWorld._42 = PlayerLocation.y;
    PlayerWorld._43 = PlayerLocation.z;

    D3DXVECTOR3 vEyePt = CameraLocation + -Forward * 10.f;
    D3DXVECTOR3 vLookatPt = PlayerLocation; 
    D3DXVECTOR3 vUpVec = Up;
    /// 월드 행렬 설정
    D3DXMATRIXA16 matWorld;
    D3DXMatrixIdentity(&matWorld);

    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
    g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

    /// 프로젝션 행렬 설정
    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 1000.0f);
    g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);

    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
  
    g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_STENCIL | D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        0xff78ffff, 1.f, 0);

    g_pd3dDevice->BeginScene();
    Matrix CubeWorld;
    D3DXMatrixIdentity(&CubeWorld);
    CubeWorld._11 *= 3.f;
    CubeWorld._22*= 3.f;
    CubeWorld._33 *= 3.f;

    CubeWorld._41 = vEyePt.x;
    CubeWorld._42 = vEyePt.y;
    CubeWorld._43 = vEyePt.z;

    g_pd3dDevice->SetTransform(D3DTS_WORLD, &CubeWorld);
    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);

    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, GetGraphic().GetCaps().MaxAnisotropy);
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    g_pd3dDevice->SetStreamSource(0, CubeMapVertexBuffer, 0, sizeof(Location3DUV));
    g_pd3dDevice->SetFVF(Location3DUV::FVF);
    g_pd3dDevice->SetIndices(CubeMapIndexBuffer);
    g_pd3dDevice->SetTexture(0, CubeTexture);
    g_pd3dDevice->SetVertexShader(nullptr);
    g_pd3dDevice->SetPixelShader(nullptr);
    g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0,
        8, 0, 12);

    g_pd3dDevice->SetTransform(D3DTS_WORLD, &PlayerWorld);
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    
    g_pd3dDevice->SetTexture(0, PlayerTexture);
    g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0,
        8, 0, 12);






    g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

    g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
    g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
    g_pd3dDevice->SetFVF(CUSTOMVERTEX::FVF);
    g_pd3dDevice->SetIndices(g_pIB);
    g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, g_cxHeight * g_czHeight, 0, (g_cxHeight - 1) * (g_czHeight - 1) * 2);



    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present(nullptr, nullptr, NULL, nullptr);
}
