#include "HierarchyLoader.h"

void Engine::Camera::Initialize(IDirect3DDevice9* const Device, const std::filesystem::path& Path)&
{
	this->Device = Device;
	this->Path = Path;
}

STDMETHODIMP_(HRESULT __stdcall) Engine::Camera::CreateFrame(LPCSTR Name, LPD3DXFRAME* NewFrame)
{
	D3DXFRAME_DERIVED* DerivedFrame;
}

STDMETHODIMP_(HRESULT __stdcall) Engine::Camera::CreateMeshContainer(LPCSTR Name, const D3DXMESHDATA* MeshData, const D3DXMATERIAL* Materials, const D3DXEFFECTINSTANCE* EffectInstances, DWORD NumMaterials, const DWORD* Adjacency, LPD3DXSKININFO SkinInfo, LPD3DXMESHCONTAINER* NewMeshContainer)
{
	return E_NOTIMPL;
}

STDMETHODIMP_(HRESULT __stdcall) Engine::Camera::DestroyFrame(LPD3DXFRAME FrameToFree)
{
	return E_NOTIMPL;
}

STDMETHODIMP_(HRESULT __stdcall) Engine::Camera::DestroyMeshContainer(LPD3DXMESHCONTAINER MeshContainerToFree)
{
	return E_NOTIMPL;
}

void Engine::Camera::AllocateName(const std::vector<std::wstring>& Names, const std::wstring FrameName)&
{
}
