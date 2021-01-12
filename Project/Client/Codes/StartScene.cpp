#include "..\\stdafx.h"
#include "StartScene.h"
#include "Management.h"
#include "EnemyLayer.h"
#include "HeightMap.h"
#include "ExportUtility.hpp"
#include "Controller.h"
#include "App.h"



LPDIRECT3DTEXTURE9  g_pTexDiffuse = nullptr; /// Texture �����
LPDIRECT3DTEXTURE9      g_pTexHeightMap = nullptr;  // HeightMap�� ����� �ؽ�ó
UINT g_cxHeight;
IDirect3DVertexBuffer9* g_pVB = nullptr;
IDirect3DDevice9* g_pd3dDevice;
LPDIRECT3DINDEXBUFFER9 g_pIB = nullptr; /// �ε����� ������ �ε�������

    /// �� ����� ����

UINT g_czHeight;
struct CUSTOMVERTEX
{
    Vector3 Location;
    static inline const DWORD FVF = D3DFVF_XYZ;
};
struct MYINDEX {
    WORD _0, _1, _2;  /// WORD, 16��Ʈ �ε���
};
/**-----------------------------------------------------------------------------
* �ؽ�ó �ʱ�ȭ
*------------------------------------------------------------------------------
*/
HRESULT InitTexture()
{
    g_pd3dDevice = GetGraphic().GetDevice().get();

    // ���̸�
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


    // D3D���� �ؽ�ó�� ���� ������ �����ϱ� ���� ����ü
    D3DSURFACE_DESC ddsd;
    // �ؽ�ó �޸��� �����͸� �����ϱ� ���� ����ü
    D3DLOCKED_RECT d3drc;

    // �ؽ�ó ������ �о�´�.
    g_pTexHeightMap->GetLevelDesc(0, &ddsd);
    // �ؽ�ó�� ����, ���� ũ�⸦ �����Ѵ�.
    g_cxHeight = ddsd.Width;
    g_czHeight = ddsd.Height;

    // �ؽ�ó�� ũ�⿡ �°� ���� ���۸� �����Ѵ�.
    if (FAILED(g_pd3dDevice->CreateVertexBuffer(g_cxHeight * g_czHeight * sizeof(CUSTOMVERTEX),
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

    // ������ ��ǥ���� �Ҵ��ϴ� ������ �˰���
    // (0, 0, 0)�� ��ġ�� ������ �����߽����� �ϱ� ���� �۾�
    CUSTOMVERTEX v;
    CUSTOMVERTEX* pV = (CUSTOMVERTEX*)pVertexBuffer;

    for (DWORD z = 0; z < g_cxHeight; ++z) {
        for (DWORD x = 0; x < g_czHeight; ++x) {
            // ������ ��ǥ���� �����Ѵ�.
            v.Location.x = (float)x - g_cxHeight / 2.f;
            // �ؽ�ó�� ���� �� ��ϰ��� ����� �޸� �ּҿ� �����Ͽ�,
            // �ؽ�ó�� ���� �� ��ϰ��� 0x000000ff�� & �����ϸ�
            // ��ϰ��� ��� �� �� �ִ�.
            // �� ���� 0 ~ 255 �����̴�.
            // �� �ڵ忡���� �� ���� 10���� ����� ���̰����� �����Ѵ�.
            v.Location.y = ((float)(*((LPDWORD)d3drc.pBits + x + z * (d3drc.Pitch / 4)) & 0x000000ff)) / 10.f;
            v.Location.z = -((float)z - g_czHeight / 2.f);

            *pV++ = v;
        }
    }

    // Lock() - Unlock()
    g_pVB->Unlock();
    g_pTexHeightMap->UnlockRect(0);

    // �ε��� ���۸� �����Ѵ�.
  // (���� * ����) ������ŭ�� �簢���� �ʿ��ϴ�.
  // ������ �簢���� 2���� �ﰢ������ �̷������.
  // �׷��� �ε��� ������ ũ��� ( g_cxHeight - 1 )*( g_czHeight - 1 ) * 2 * sizeof( MYINDEX )
    if (FAILED(g_pd3dDevice->CreateIndexBuffer((g_cxHeight - 1) * (g_czHeight - 1) * 2 * sizeof(MYINDEX),
        0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pIB, NULL))) {
        return E_FAIL;
    }

    // �ε��� ���ۿ� ������ �ε����� �����ϴ� �˰���
    MYINDEX  i;
    MYINDEX* pI;
    if (FAILED(g_pIB->Lock(0, (g_cxHeight - 1) * (g_czHeight - 1) * 2 * sizeof(MYINDEX), (void**)&pI, 0)))
        return E_FAIL;

    for (DWORD z = 0; z < g_czHeight - 1; z++) {
        for (DWORD x = 0; x < g_cxHeight - 1; x++) {

            // ���� �ε����� CW �������� �����ϰ� �ִ�.
            // ���� CW �������� �ø� ��带 �����ϸ� �ƹ��͵� �׷����� ���� ���̴�.
            // g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW ) �� ��� 
            // �׷����� ���� ����.

            // �»� ��ġ�� �ﰢ��
            i._0 = (z * g_cxHeight + x);
            i._1 = (z * g_cxHeight + x + 1);
            i._2 = ((z + 1) * g_cxHeight + x);
            // ������ ������ ���� ���� �ε��� ������ġ ����
            *pI++ = i;

            // ���� ��ġ�� �ﰢ��
            i._0 = ((z + 1) * g_cxHeight + x);
            i._1 = (z * g_cxHeight + x + 1);
            i._2 = ((z + 1) * g_cxHeight + x + 1);
            // ������ ������ ���� ���� �ε��� ������ġ ����
            *pI++ = i;
        }
    }
    // Lock() - Unlock()
    g_pIB->Unlock();



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
float Speed = 100.f;
Vector3 Up{ 0,1,0 };
Vector3 Right{ 1,0,0 };
void StartScene::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);

    if (GetAsyncKeyState('W') & 0x8000)
    {
        CameraLocation += Forward * Speed * DeltaTime;
    }
    if (GetAsyncKeyState('S') & 0x8000)
    {
        CameraLocation += -Forward * Speed * DeltaTime;
    }
    if (GetAsyncKeyState('D') & 0x8000)
    {
        CameraLocation += Right* Speed * DeltaTime;
    }
    if (GetAsyncKeyState('A') & 0x8000)
    {
        CameraLocation += -Right* Speed * DeltaTime;
    }

    auto ClientSize = App::Instance->GetClientSize();
    HWND _Hwnd = App::Instance->GetHwnd();
    POINT Pt;
    GetCursorPos(&Pt);
    ScreenToClient(_Hwnd, &Pt);
    
    float dx = ((float)Pt.x - (ClientSize.first/2) );
    float dy = ((float)Pt.y - (ClientSize.second/2 ) );
    dx = D3DXToRadian(dx)*0.3f;
    dy = D3DXToRadian(dy) * 0.3f;

    Matrix rotx, roty;
    D3DXMatrixRotationAxis(&rotx, &Up, dx);
    D3DXVec3TransformNormal(&Forward, &Forward, &rotx);
    D3DXVec3TransformNormal(&Right, &Right, &rotx);

    D3DXMatrixRotationAxis(&roty, &Right, dy);
    D3DXVec3TransformNormal(&Forward, &Forward, &roty);
    D3DXVec3TransformNormal(&Up, &Up, &roty);
    Pt.x = ClientSize.first / 2;
    Pt.y = ClientSize.second / 2;
    ClientToScreen(_Hwnd, &Pt);
    SetCursorPos(Pt.x, Pt.y);


    D3DXVECTOR3 vEyePt = CameraLocation;
    D3DXVECTOR3 vLookatPt = CameraLocation + Forward * 1.f;
    D3DXVECTOR3 vUpVec = Up;
    /// ���� ��� ����
    D3DXMATRIXA16 matWorld;
    D3DXMatrixIdentity(&matWorld);
    g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
    g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

    /// �������� ��� ����
    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 1000.0f);
    g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);

    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

    g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_STENCIL | D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        0xff78ffff, 1.f, 0);

    g_pd3dDevice->BeginScene();
    g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
    g_pd3dDevice->SetFVF(CUSTOMVERTEX::FVF);
    g_pd3dDevice->SetIndices(g_pIB);
    g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, g_cxHeight * g_czHeight, 0, (g_cxHeight - 1) * (g_czHeight - 1) * 2);



    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present(nullptr, nullptr, NULL, nullptr);
}
