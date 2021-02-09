#include "Management.h"
#include "Timer.h"
#include "NavigationMesh.h"
#include "Timer.h"

#include <chrono>
#include "Controller.h"
#include "Sound.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "PrototypeManager.h"
#include "FontManager.h"
#include "FMath.hpp"
#include "ResourceSystem.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "imgui.h"
#include "Vertexs.hpp"
#include "CollisionSystem.h"
#include "UtilityGlobal.h"

bool DebugMode{ false };

void ImGuiInitialize(const HWND Hwnd, IDirect3DDevice9* const Device);
void ImGuiFrameStart();

static inline void  SetupImGuiStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;
	
	style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_::ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style->Colors[ImGuiCol_::ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
	

}


void Engine::Management::Initialize(
	const HWND _Hwnd,
	const HINSTANCE HInstance,
	const bool bFullScreen,
	const std::pair<uint32, uint32> ClientSize,
	const D3DMULTISAMPLE_TYPE MultiSample,
	const float DeltaMax,
	const std::filesystem::path& ResourcePath)&
{
	Hwnd = _Hwnd;
	this->ClientSize = ClientSize;
	this->ResourcePath = ResourcePath;

	_GraphicDevice = Engine::GraphicDevice::Init(
		_Hwnd,
		bFullScreen,
		ClientSize,
		MultiSample);

	_Timer = Engine::Timer::Init(
		DeltaMax,
		[this]() {Event(); },
		[this](const float DeltaTime) {Update(DeltaTime); },
		[this]() {Render(); },
		[this]() {LastEvent(); });

	_Sound = Engine::Sound::Init(ResourcePath / L"Sound");
	_Controller = Engine::Controller::Init(HInstance, _Hwnd);
	_CollisionSys = Engine::CollisionSystem::Init();
	auto SharedDevice = _GraphicDevice->GetDevice();
	Device = SharedDevice.get();
	_ShaderManager = Engine::ShaderManager::Init(SharedDevice);
	_PrototypeManager = Engine::PrototypeManager::Init();
	_FontManager = Engine::FontManager::Init();
	_ResourceSys = Engine::ResourceSystem::Init();
	_Renderer = Engine::Renderer::Init(SharedDevice);
	_NaviMesh = Engine::NavigationMesh::Init(Device);

	ImGuiInitialize(Hwnd, Device);
	Engine::Global::Hwnd = Hwnd;
	Engine::Global::ClientSize = ClientSize;
	CreateStaticResource();
	CreateCollisionDebugResource();
}

Engine::Management::~Management() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	if (_CurrentScene)
		_CurrentScene.reset();

	Sound::Reset();
	Controller::Reset();
	Timer::Reset();
	NavigationMesh::Reset();
	CollisionSystem::Reset();
	PrototypeManager::Reset();
	Renderer::Reset();
	ShaderManager::Reset();
	FontManager::Reset();
	ResourceSystem::Reset();
	GraphicDevice::Reset();
};



void Engine::Management::GameLoop()&
{
	_Timer->Update();
}

void Engine::Management::Event()&
{
	ImGuiFrameStart();

	_Controller->Update();
	_Sound->Update();
	_CurrentScene->Event();
}

void Engine::Management::Update(const float DeltaTime)&
{
	_CurrentScene->Update(DeltaTime);
	_CollisionSys->Update(DeltaTime);
}

void Engine::Management::Render()&
{
	Matrix InvView;
	_GraphicDevice->GetDevice()->GetTransform(D3DTS_VIEW, &InvView);
	InvView = FMath::Inverse(InvView);
	const Vector3 LightLocation{ 0,0,0 };
	const Vector3 CameraLocation{ InvView._41,InvView._42,InvView._43 };
	_ShaderManager->Update(CameraLocation, LightLocation);
	_GraphicDevice->Begin();
	_Renderer->Render();
	_NaviMesh->Render(Device);
	_CurrentScene->Render();
	// 폰트 드로우콜
	{
	/*	_FontManager->RenderFont(L"Font_Jinji", L"진지함", { 400,300 }, D3DXCOLOR{ 0.5f,1.f,0.5f,0.1f });
		_FontManager->RenderFont(L"Font_Default", L"기본", { 600,200 }, D3DXCOLOR{ 0.5f,0.f,0.5f,1.f });*/
	}

	if (Engine::Global::bDebugMode)
	{
		std::wstring TimerInfoW = _Timer->Information;
		std::string TimerInfoA;
		TimerInfoA.assign(std::begin(TimerInfoW), std::end(TimerInfoW));
		if (ImGui::CollapsingHeader("FrameRate"))
		{
			ImGui::Text(TimerInfoA.c_str());
		}
	}
	

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	_GraphicDevice->End();
}

void Engine::Management::LastEvent()&
{
	_CurrentScene->PendingKill();

	if (SceneChangeEvent)
	{
		SceneChangeEvent();
		SceneChangeEvent = nullptr;
	}
}

void ImGuiInitialize(const HWND Hwnd, IDirect3DDevice9* const Device)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& ImGuiIoRef = ImGui::GetIO();
	ImGuiIoRef.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGuiIoRef .Fonts->AddFontFromFileTTF("..\\..\\..\\Resource\\Font\\Ruda\\static\\Ruda-Bold.ttf", 12);
	ImGuiIoRef .Fonts->AddFontFromFileTTF(("..\\..\\..\\Resource\\Font\\Ruda\\static\\Ruda-Bold.ttf"), 10);
	ImGuiIoRef .Fonts->AddFontFromFileTTF(("..\\..\\..\\Resource\\Font\\Ruda\\static\\Ruda-Bold.ttf"), 14);
	ImGuiIoRef .Fonts->AddFontFromFileTTF(("..\\..\\..\\Resource\\Font\\Ruda\\static\\Ruda-Bold.ttf"), 18);

	ImGui::StyleColorsDark();
	SetupImGuiStyle();
	ImGui_ImplWin32_Init(Hwnd);
	ImGui_ImplDX9_Init(Device);
}

void ImGuiFrameStart()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void Engine::Management::CreateStaticResource()&
{
	// 큐브용 인덱스 버퍼 로딩.
	auto Device = _GraphicDevice->GetDevice();

	IDirect3DIndexBuffer9* CubeIdxBuffer{ nullptr };
	Device->CreateIndexBuffer(sizeof(Index::_16) * 12u,
		0,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&CubeIdxBuffer, nullptr);
	_ResourceSys->Insert<IDirect3DIndexBuffer9>
		(L"IndexBuffer_Cube", CubeIdxBuffer);

	Index::_16* IndexBufferPtr{ nullptr };
	CubeIdxBuffer->Lock(0, 0, (void**)&IndexBufferPtr, 0);

	// x+
	IndexBufferPtr[0]._1 = 1;
	IndexBufferPtr[0]._2 = 5;
	IndexBufferPtr[0]._3 = 6;

	IndexBufferPtr[1]._1 = 1;
	IndexBufferPtr[1]._2 = 6;
	IndexBufferPtr[1]._3 = 2;

	// x-
	IndexBufferPtr[2]._1 = 4;
	IndexBufferPtr[2]._2 = 0;
	IndexBufferPtr[2]._3 = 3;

	IndexBufferPtr[3]._1 = 4;
	IndexBufferPtr[3]._2 = 3;
	IndexBufferPtr[3]._3 = 7;

	// y+
	IndexBufferPtr[4]._1 = 4;
	IndexBufferPtr[4]._2 = 5;
	IndexBufferPtr[4]._3 = 1;

	IndexBufferPtr[5]._1 = 4;
	IndexBufferPtr[5]._2 = 1;
	IndexBufferPtr[5]._3 = 0;

	// y-
	IndexBufferPtr[6]._1 = 3;
	IndexBufferPtr[6]._2 = 2;
	IndexBufferPtr[6]._3 = 6;

	IndexBufferPtr[7]._1 = 3;
	IndexBufferPtr[7]._2 = 6;
	IndexBufferPtr[7]._3 = 7;

	// z+
	IndexBufferPtr[8]._1 = 7;
	IndexBufferPtr[8]._2 = 6;
	IndexBufferPtr[8]._3 = 5;

	IndexBufferPtr[9]._1 = 7;
	IndexBufferPtr[9]._2 = 5;
	IndexBufferPtr[9]._3 = 4;

	// z-
	IndexBufferPtr[10]._1 = 0;
	IndexBufferPtr[10]._2 = 1;
	IndexBufferPtr[10]._3 = 2;

	IndexBufferPtr[11]._1 = 0;
	IndexBufferPtr[11]._2 = 2;
	IndexBufferPtr[11]._3 = 3;

	CubeIdxBuffer->Unlock();

	IDirect3DVertexBuffer9* FrustumVertexBuffer{ nullptr };
	IDirect3DIndexBuffer9* FrustumIndexBuffer{ nullptr };

	Device->CreateVertexBuffer(
		8 * sizeof(Vector3),
		D3DUSAGE_WRITEONLY,
		D3DFVF_XYZ,
		D3DPOOL_MANAGED,
		&FrustumVertexBuffer,
		0);

	Device->CreateIndexBuffer(
		36 * sizeof(WORD),
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&FrustumIndexBuffer,
		0);

	Vector3* Vertices;
	FrustumVertexBuffer->Lock(0, 0, (void**)&Vertices, 0);

	Vertices[0] = Vector3(-1.0f, -1.0f, 0.f);
	Vertices[1] = Vector3(-1.0f, 1.0f, 0.f);
	Vertices[2] = Vector3(1.0f, 1.0f, 0.f);
	Vertices[3] = Vector3(1.0f, -1.0f, 0.f);
	Vertices[4] = Vector3(-1.0f, -1.0f, 1.0f);
	Vertices[5] = Vector3(-1.0f, 1.0f, 1.0f);
	Vertices[6] = Vector3(1.0f, 1.0f, 1.0f);
	Vertices[7] = Vector3(1.0f, -1.0f, 1.0f);

	FrustumVertexBuffer->Unlock();

	WORD* Indices = 0;
	FrustumIndexBuffer->Lock(0, 0, (void**)&Indices, 0);

	// front side
	Indices[0] = 0; Indices[1] = 1; Indices[2] = 2;
	Indices[3] = 0; Indices[4] = 2; Indices[5] = 3;

	// back side
	Indices[6] = 4; Indices[7] = 6; Indices[8] = 5;
	Indices[9] = 4; Indices[10] = 7; Indices[11] = 6;

	// left side
	Indices[12] = 4; Indices[13] = 5; Indices[14] = 1;
	Indices[15] = 4; Indices[16] = 1; Indices[17] = 0;

	// right side
	Indices[18] = 3; Indices[19] = 2; Indices[20] = 6;
	Indices[21] = 3; Indices[22] = 6; Indices[23] = 7;

	// top
	Indices[24] = 1; Indices[25] = 5; Indices[26] = 6;
	Indices[27] = 1; Indices[28] = 6; Indices[29] = 2;

	// bottom
	Indices[30] = 4; Indices[31] = 0; Indices[32] = 3;
	Indices[33] = 4; Indices[34] = 3; Indices[35] = 7;

	FrustumIndexBuffer->Unlock();

	_ResourceSys->Insert<IDirect3DVertexBuffer9>
		(L"VertexBuffer_Frustum", FrustumVertexBuffer);
	_ResourceSys->Insert<IDirect3DIndexBuffer9>
		(L"IndexBuffer_Frustum", FrustumIndexBuffer);

	/*ID3DXLine* _DxLine{ nullptr };
	_ResourceSys->Emplace<ID3DXLine>(L"Line", D3DXCreateLine,
		Device.get(), &_DxLine);*/
	ID3DXMesh* SphereMesh{ nullptr };
	ID3DXBuffer* SphereMeshAdjacency{ nullptr };
	D3DXCreateSphere(Device.get(), 5.f, 10, 10, &SphereMesh, &SphereMeshAdjacency);
	_ResourceSys->Insert<ID3DXMesh>(L"SphereMesh", SphereMesh);
	_ResourceSys->Insert<ID3DXBuffer>(L"SphereMeshAdjacency", SphereMeshAdjacency);

	// UI 용 사각형
	{
		IDirect3DVertexBuffer9* VertexBuffer{ nullptr }; 
		Device->CreateVertexBuffer(sizeof(Vertex::LocationUV2D) * 6u, D3DUSAGE_WRITEONLY,
			Vertex::LocationUV2D::FVF, D3DPOOL_MANAGED, &VertexBuffer, nullptr);
		Vertex::LocationUV2D* VtxBufPtr{};
		VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VtxBufPtr), NULL);
		VtxBufPtr[0].Location = { -0.5f,0.5f,0.f };
		VtxBufPtr[0].UV = { 0.0f,0.0f };
		VtxBufPtr[1].Location = { 0.5f,0.5f,0.f };
		VtxBufPtr[1].UV = { 1.f,0.f };
		VtxBufPtr[2].Location = { -0.5f,-0.5f,0.f };
		VtxBufPtr[2].UV = { 0.f,1.f };
		VtxBufPtr[3].Location = { -0.5f,-0.5f,0.f };
		VtxBufPtr[3].UV = { 0.f,1.f };
		VtxBufPtr[4].Location = { 0.5f,0.5f,0.f };
		VtxBufPtr[4].UV = { 1.f,0.f };
		VtxBufPtr[5].Location = { 0.5f,-0.5f,0.f };
		VtxBufPtr[5].UV = { 1.f,1.f };
		VertexBuffer->Unlock();
		_ResourceSys->Insert<IDirect3DVertexBuffer9>(L"VertexBuffer_Plane", VertexBuffer);
	}
	
	{
		// 폰트 로딩
		_FontManager->AddFont(Device.get(), L"Font_Default", L"바탕", 15, 20, FW_HEAVY);
		_FontManager->AddFont(Device.get(), L"Font_Jinji", L"궁서", 15, 20, FW_THIN);
	}
}
void Engine::Management::CreateCollisionDebugResource()&
{
	auto Device = _GraphicDevice->GetDevice();

	IDirect3DTexture9* TextureNoCollision{ nullptr };
	IDirect3DTexture9* TextureCollision{ nullptr };

	{
		Device->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &TextureNoCollision, NULL);
		D3DLOCKED_RECT		LockRect;
		ZeroMemory(&LockRect, sizeof(D3DLOCKED_RECT));
		TextureNoCollision->LockRect(0, &LockRect, NULL, 0);
		*((uint32*)LockRect.pBits) = D3DXCOLOR(1.f, 0.f, 0.f, 1.f);
		TextureNoCollision->UnlockRect(0);
		_ResourceSys->Insert<IDirect3DTexture9>(L"Texture_Collision", TextureNoCollision);
	}

	{
		Device->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &TextureCollision, NULL);
		D3DLOCKED_RECT		LockRect;
		ZeroMemory(&LockRect, sizeof(D3DLOCKED_RECT));
		TextureCollision->LockRect(0, &LockRect, NULL, 0);
		*((uint32*)LockRect.pBits) = D3DXCOLOR(0.f, 1.f, 0.f, 1.f);
		TextureCollision->UnlockRect(0);
		_ResourceSys->Insert<IDirect3DTexture9>(L"Texture_NoCollision", TextureCollision);
	}
};
