#include "HeightMap.h"
#include "imgui.h"
#include "ResourceSystem.h"
#include "Vertexs.hpp"
#include "Transform.h"
#include "Management.h"
#include "FMath.hpp"
#include "GraphicDevice.h"

LPDIRECT3DTEXTURE9      g_pTexHeightMap = nullptr;  // HeightMap에 사용할 텍스처
UINT g_cxHeight;
IDirect3DVertexBuffer9* g_pVB = nullptr;
LPDIRECT3DINDEXBUFFER9 g_pIB = nullptr; /// 인덱스를 보관할 인덱스버퍼
UINT g_czHeight;
struct CUSTOMVERTEX
{
    Vector3 Location;
    static inline const DWORD FVF = D3DFVF_XYZ;
};

struct MYINDEX {
    WORD _0, _1, _2;  /// WORD, 16비트 인덱스
};

void Temp()
{
  //  auto g_pd3dDevice = Engine::GraphicDevice::Instance->GetDevice().get();

  //  // 높이맵
  //  if (FAILED(D3DXCreateTextureFromFileEx(g_pd3dDevice,
  //      L"..\\..\\..\\Resource\\Texture\\Terrain\\Height2.bmp", D3DX_DEFAULT, D3DX_DEFAULT,
  //      D3DX_DEFAULT, 0,
  //      D3DFMT_X8R8G8B8, D3DPOOL_MANAGED,
  //      D3DX_DEFAULT, D3DX_DEFAULT, 0,
  //      NULL, NULL, &g_pTexHeightMap
  //  ))) {
  //      MessageBox(NULL, L"Can't not open the texture file.", L"Error", MB_OK);

  //  }


  //  // D3D에서 텍스처에 대한 정보를 저장하기 위한 구조체
  //  D3DSURFACE_DESC ddsd;
  //  // 텍스처 메모리의 포인터를 저장하기 위한 구조체
  //  D3DLOCKED_RECT d3drc;

  //  // 텍스처 정보를 읽어온다.
  //  g_pTexHeightMap->GetLevelDesc(0, &ddsd);
  //  // 텍스처의 가로, 세로 크기를 저장한다.
  //  g_cxHeight = ddsd.Width;
  //  g_czHeight = ddsd.Height;

  //  // 텍스처의 크기에 맞게 정점 버퍼를 생성한다.
  //  if (FAILED(g_pd3dDevice->CreateVertexBuffer(g_cxHeight * g_czHeight *
  //      sizeof(CUSTOMVERTEX),
  //      0, CUSTOMVERTEX::FVF, D3DPOOL_DEFAULT, &g_pVB, NULL))) {
  //      MessageBox(NULL, L"Can't not open the texture file.", L"Error", MB_OK);

  //  }

  //  if (FAILED(g_pTexHeightMap->LockRect(0, &d3drc, NULL, D3DLOCK_READONLY)))
  //  {
  //      MessageBox(NULL, L"Can't not open the texture file.", L"Error", MB_OK);

  //  }
  //  VOID* pVertexBuffer = nullptr;
  //  if (FAILED(g_pVB->Lock(0, g_cxHeight * g_czHeight * sizeof(CUSTOMVERTEX), &pVertexBuffer, 0))) {
  //      MessageBox(NULL, L"Can't not open the texture file.", L"Error", MB_OK);

  //  }

  //  // 정점의 좌표값을 할당하는 간단한 알고리즘
  //  // (0, 0, 0)의 위치를 지형의 무게중심으로 하기 위한 작업
  //  CUSTOMVERTEX v;
  //  CUSTOMVERTEX* pV = (CUSTOMVERTEX*)pVertexBuffer;

  //  for (DWORD z = 0; z < g_cxHeight; ++z) {
  //      for (DWORD x = 0; x < g_czHeight; ++x) {
  //          // 정점의 좌표값을 설정한다.
  //          v.Location.x = (float)x - g_cxHeight / 2.f;
  //          // 텍스처의 색상 및 명암값이 저장된 메모리 주소에 접근하여,
  //          // 텍스처의 색상 및 명암값과 0x000000ff와 & 연산하면
  //          // 명암값을 얻어 올 수 있다.
  //          // 이 값은 0 ~ 255 사이이다.
  //          // 이 코드에서는 이 값을 10으로 나누어서 높이값으로 설정한다.
  //          v.Location.y = ((float)(*((LPDWORD)d3drc.pBits + x + z * (d3drc.Pitch / 4)) & 0x000000ff)) / 10.f;
  //          v.Location.z = -((float)z - g_czHeight / 2.f);

  //          *pV++ = v;
  //      }
  //  }

  //  // Lock() - Unlock()

  //  g_pTexHeightMap->UnlockRect(0);

  //  // 인덱스 버퍼를 생성한다.
  //// (가로 * 세로) 갯수만큼의 사각형이 필요하다.
  //// 각각의 사각형은 2개의 삼각형으로 이루어진다.
  //// 그래서 인덱스 버퍼의 크기는 ( g_cxHeight - 1 )*( g_czHeight - 1 ) * 2 * sizeof( MYINDEX )
  //  if (FAILED(g_pd3dDevice->CreateIndexBuffer((g_cxHeight - 1) * (g_czHeight - 1) * 2 * sizeof(MYINDEX),
  //      0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pIB, NULL))) {
  //      MessageBox(NULL, L"Can't not open the texture file.", L"Error", MB_OK);

  //  }

  //  // 인덱스 버퍼에 저정할 인덱스를 생성하는 알고리즘
  //  MYINDEX  i;
  //  MYINDEX* pI;
  //  if (FAILED(g_pIB->Lock(0, (g_cxHeight - 1) * (g_czHeight - 1) * 2 * sizeof(MYINDEX), (void**)&pI, 0)))
  //  {
  //      MessageBox(NULL, L"Can't not open the texture file.", L"Error", MB_OK);

  //  }
  //  pV = (CUSTOMVERTEX*)pVertexBuffer;
  //  for (DWORD z = 0; z < g_czHeight - 1; z++) {
  //      for (DWORD x = 0; x < g_cxHeight - 1; x++) {

  //          // 정점 인덱스를 CW 방향으로 설정하고 있다.
  //          // 만약 CW 방향으로 컬링 모드를 설정하면 아무것도 그려지지 않을 것이다.
  //          // g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW ) 인 경우 
  //          // 그려지는 것이 없다.

  //          // 좌상 위치의 삼각형
  //          i._0 = (z * g_cxHeight + x);
  //          i._1 = (z * g_cxHeight + x + 1);
  //          i._2 = ((z + 1) * g_cxHeight + x);


  //          // 포인터 연산을 통해 다음 인덱스 저장위치 설정
  //          *pI++ = i;

  //          // 우하 위치의 삼각형
  //          i._0 = ((z + 1) * g_cxHeight + x);
  //          i._1 = (z * g_cxHeight + x + 1);
  //          i._2 = ((z + 1) * g_cxHeight + x + 1);


  //          // 포인터 연산을 통해 다음 인덱스 저장위치 설정
  //          *pI++ = i;
  //      }
  //  }


  //  g_pVB->Unlock();
  //  // Lock() - Unlock()
  //  g_pIB->Unlock();
}



void Engine::HeightMap::Initialize()&
{
    Super::Initialize();

    AddComponent<Transform>();
}

void Engine::HeightMap::PrototypeInitialize(
    IDirect3DDevice9* const Device,
    const RenderInterface::Group _Group)&
{
    Super::PrototypeInitialize(Device, _Group);

    auto& ResourceSys = ResourceSystem::Instance;
    ResourceSys->Emplace<IDirect3DTexture9>(L"MapTexture",
        D3DXCreateTextureFromFile, Device, L"..\\..\\..\\Resource\\Texture\\Player1.jpg", &Texture);

    Device->CreateVertexBuffer(sizeof(Vertex::Texture) * 3u, D3DUSAGE_DYNAMIC,
        Vertex::Texture::FVF, D3DPOOL_DEFAULT, &VertexBuffer, nullptr);
    ResourceSys->Insert<IDirect3DVertexBuffer9>(L"VertexBuffer", VertexBuffer);

    Vertex::Texture* ptr;
    VertexBuffer->Lock(0, sizeof(Vertex::Texture) * 3u, (void**)&ptr, 0);
    ptr[0].Location = { -0.5,-0.5,0.f };
    ptr[0].TexCoord = { 0,1 };
    ptr[1].Location = { 0.5,0.5,0.f };
    ptr[1].TexCoord = { 1,0 };
    ptr[2].Location = { 0.5,-0.5,0.f };
    ptr[2].TexCoord = { 1,1 };
    VertexBuffer->Unlock();


    Temp();
}

void Engine::HeightMap::Event()&
{
    Super::Event();

    ImGui::Begin("TEST");
    static int TestFloat = 1;
    ImGui::SliderInt("FLOAT", &TestFloat, 0.f, 100.f);
    ImGui::End();
}
void Engine::HeightMap::Update(const float DeltaTime)&
{
    Super::Update(DeltaTime);
}

void Engine::HeightMap::Render()&
{
    //auto _Transform = GetComponent < Transform>();

    //for (size_t i = 0; i < 10000; ++i)
    //{
    //    Vector3 RandVector = FMath::Normalize(FMath::Random(Vector3{ -1,-1,-1, }, Vector3{ 1,1,1 }));

    //    _Transform->Rotate(RandVector ,     
    //    FMath::Random(-FMath::PI, FMath::PI));
    //}

    //Device->SetFVF(Vertex::Texture::FVF);
    //
    //Device->SetTransform(D3DTS_WORLD, &_Transform->UpdateWorld());
    //Device->SetStreamSource(0, VertexBuffer, 0, sizeof(Vertex::Texture));
    ////Device->SetTexture(0, Texture);
    //Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);


    //// auto g_pd3dDevice = Engine::GraphicDevice::Instance->GetDevice().get();
    //Matrix View, Proj;
    //D3DXMatrixPerspectiveFovLH(&Proj, FMath::PI / 4.f, 1920.f / 1080.f, 1.f, 1000.f);
    //Vector3 AA = { 0,0,0 };

    //Vector3 BB = { 0,0,-10.f };
    //auto CC = Vector3{ 0,1,0 };
    //D3DXMatrixLookAtLH(&View, &BB, &AA,
    //    &CC);
    //Matrix World;
    //World = _Transform->UpdateWorld();

    ///*Device->SetTransform(D3DTS_VIEW, &View);
    //Device->SetTransform(D3DTS_PROJECTION, &Proj);*/
    //Device->SetTransform(D3DTS_WORLD, &World);

    //Device->SetRenderState(D3DRS_LIGHTING, FALSE);
    //Device->SetRenderState(D3DRS_ZENABLE, FALSE);
    //Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    //Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    //Device->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
    //Device->SetFVF(CUSTOMVERTEX::FVF);
    //Device->SetIndices(g_pIB);
    //Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, g_cxHeight * g_czHeight, 0, (g_cxHeight - 1) * (g_czHeight - 1) * 2);
    ////Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    ////Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

}