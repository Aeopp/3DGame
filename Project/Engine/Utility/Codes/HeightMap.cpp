#include "HeightMap.h"
#include "imgui.h"
#include "ResourceSystem.h"
#include "Vertexs.hpp"
#include "Transform.h"
#include "Management.h"
#include "FMath.hpp"
#include "GraphicDevice.h"

LPDIRECT3DTEXTURE9      g_pTexHeightMap = nullptr;  // HeightMap�� ����� �ؽ�ó
UINT g_cxHeight;
IDirect3DVertexBuffer9* g_pVB = nullptr;
LPDIRECT3DINDEXBUFFER9 g_pIB = nullptr; /// �ε����� ������ �ε�������
UINT g_czHeight;
struct CUSTOMVERTEX
{
    Vector3 Location;
    static inline const DWORD FVF = D3DFVF_XYZ;
};

struct MYINDEX {
    WORD _0, _1, _2;  /// WORD, 16��Ʈ �ε���
};

void Temp()
{
  //  auto g_pd3dDevice = Engine::GraphicDevice::Instance->GetDevice().get();

  //  // ���̸�
  //  if (FAILED(D3DXCreateTextureFromFileEx(g_pd3dDevice,
  //      L"..\\..\\..\\Resource\\Texture\\Terrain\\Height2.bmp", D3DX_DEFAULT, D3DX_DEFAULT,
  //      D3DX_DEFAULT, 0,
  //      D3DFMT_X8R8G8B8, D3DPOOL_MANAGED,
  //      D3DX_DEFAULT, D3DX_DEFAULT, 0,
  //      NULL, NULL, &g_pTexHeightMap
  //  ))) {
  //      MessageBox(NULL, L"Can't not open the texture file.", L"Error", MB_OK);

  //  }


  //  // D3D���� �ؽ�ó�� ���� ������ �����ϱ� ���� ����ü
  //  D3DSURFACE_DESC ddsd;
  //  // �ؽ�ó �޸��� �����͸� �����ϱ� ���� ����ü
  //  D3DLOCKED_RECT d3drc;

  //  // �ؽ�ó ������ �о�´�.
  //  g_pTexHeightMap->GetLevelDesc(0, &ddsd);
  //  // �ؽ�ó�� ����, ���� ũ�⸦ �����Ѵ�.
  //  g_cxHeight = ddsd.Width;
  //  g_czHeight = ddsd.Height;

  //  // �ؽ�ó�� ũ�⿡ �°� ���� ���۸� �����Ѵ�.
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

  //  // ������ ��ǥ���� �Ҵ��ϴ� ������ �˰���
  //  // (0, 0, 0)�� ��ġ�� ������ �����߽����� �ϱ� ���� �۾�
  //  CUSTOMVERTEX v;
  //  CUSTOMVERTEX* pV = (CUSTOMVERTEX*)pVertexBuffer;

  //  for (DWORD z = 0; z < g_cxHeight; ++z) {
  //      for (DWORD x = 0; x < g_czHeight; ++x) {
  //          // ������ ��ǥ���� �����Ѵ�.
  //          v.Location.x = (float)x - g_cxHeight / 2.f;
  //          // �ؽ�ó�� ���� �� ��ϰ��� ����� �޸� �ּҿ� �����Ͽ�,
  //          // �ؽ�ó�� ���� �� ��ϰ��� 0x000000ff�� & �����ϸ�
  //          // ��ϰ��� ��� �� �� �ִ�.
  //          // �� ���� 0 ~ 255 �����̴�.
  //          // �� �ڵ忡���� �� ���� 10���� ����� ���̰����� �����Ѵ�.
  //          v.Location.y = ((float)(*((LPDWORD)d3drc.pBits + x + z * (d3drc.Pitch / 4)) & 0x000000ff)) / 10.f;
  //          v.Location.z = -((float)z - g_czHeight / 2.f);

  //          *pV++ = v;
  //      }
  //  }

  //  // Lock() - Unlock()

  //  g_pTexHeightMap->UnlockRect(0);

  //  // �ε��� ���۸� �����Ѵ�.
  //// (���� * ����) ������ŭ�� �簢���� �ʿ��ϴ�.
  //// ������ �簢���� 2���� �ﰢ������ �̷������.
  //// �׷��� �ε��� ������ ũ��� ( g_cxHeight - 1 )*( g_czHeight - 1 ) * 2 * sizeof( MYINDEX )
  //  if (FAILED(g_pd3dDevice->CreateIndexBuffer((g_cxHeight - 1) * (g_czHeight - 1) * 2 * sizeof(MYINDEX),
  //      0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pIB, NULL))) {
  //      MessageBox(NULL, L"Can't not open the texture file.", L"Error", MB_OK);

  //  }

  //  // �ε��� ���ۿ� ������ �ε����� �����ϴ� �˰���
  //  MYINDEX  i;
  //  MYINDEX* pI;
  //  if (FAILED(g_pIB->Lock(0, (g_cxHeight - 1) * (g_czHeight - 1) * 2 * sizeof(MYINDEX), (void**)&pI, 0)))
  //  {
  //      MessageBox(NULL, L"Can't not open the texture file.", L"Error", MB_OK);

  //  }
  //  pV = (CUSTOMVERTEX*)pVertexBuffer;
  //  for (DWORD z = 0; z < g_czHeight - 1; z++) {
  //      for (DWORD x = 0; x < g_cxHeight - 1; x++) {

  //          // ���� �ε����� CW �������� �����ϰ� �ִ�.
  //          // ���� CW �������� �ø� ��带 �����ϸ� �ƹ��͵� �׷����� ���� ���̴�.
  //          // g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW ) �� ��� 
  //          // �׷����� ���� ����.

  //          // �»� ��ġ�� �ﰢ��
  //          i._0 = (z * g_cxHeight + x);
  //          i._1 = (z * g_cxHeight + x + 1);
  //          i._2 = ((z + 1) * g_cxHeight + x);


  //          // ������ ������ ���� ���� �ε��� ������ġ ����
  //          *pI++ = i;

  //          // ���� ��ġ�� �ﰢ��
  //          i._0 = ((z + 1) * g_cxHeight + x);
  //          i._1 = (z * g_cxHeight + x + 1);
  //          i._2 = ((z + 1) * g_cxHeight + x + 1);


  //          // ������ ������ ���� ���� �ε��� ������ġ ����
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