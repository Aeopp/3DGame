#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include <filesystem>

namespace Engine
{
	struct DLL_DECL Bone : public D3DXFRAME
	{
		Matrix ToRootSpace{};
	};

	struct DLL_DECL MeshContainerSub : public D3DXMESHCONTAINER
	{
		std::vector<IDirect3DTexture9*> Textures{ };
		// 최초 로드 시점의 상태를 가지고 있는 메쉬 객체
		ID3DXMesh* OriginMesh{ nullptr };
		uint32 NumBones{ 0u };
		//               애니메이션 정보를 포함한 최초 행렬 상태.
		std::vector<Matrix>  FrameOffsetMatrix{};
		// 본 구조체가 가지고 있는 CombinedMatrix 참조
		std::vector<Matrix*> FrameCombinedMatrix;
		std::vector<Matrix> RenderingMatrix;
	};

	class DLL_DECL HierarchyLoader : public  ID3DXAllocateHierarchy
	{
	public:
		void Initialize(IDirect3DDevice9* const Device,
			const std::filesystem::path& Path)&;
	public:
		STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME* NewFrame);
		STDMETHOD(CreateMeshContainer)(THIS_ LPCSTR Name,
			CONST D3DXMESHDATA* MeshData,
			CONST D3DXMATERIAL* Materials,
			CONST D3DXEFFECTINSTANCE* EffectInstances,
			DWORD NumMaterials,
			CONST DWORD* Adjacency,
			LPD3DXSKININFO SkinInfo,
			LPD3DXMESHCONTAINER* NewMeshContainer);
		STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME FrameToFree);
		STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER MeshContainerToFree);
	private:
		void AllocateName(char** ppName, const char* pFrameName)&;
	private:
		IDirect3DDevice9*  Device{ nullptr };
		std::filesystem::path Path;
	};
};



