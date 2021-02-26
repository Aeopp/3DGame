#include "Sky.h"
#include "imgui.h"
#include "FMath.hpp"
#include "ResourceSystem.h"
#include "UtilityGlobal.h"
#include <array>
#include "AssimpHelper.h"
#include "StringHelper.h"
#include "Timer.h"
#include "Vertexs.hpp"

void Engine::Sky::Initialize(const std::filesystem::path& FullPath  , IDirect3DDevice9* const Device)&
{
	using VertexType = Vertex::LocationUV2D;
	_SkyMesh.Stride = sizeof(VertexType); 
	auto AiScene = Engine::Global::AssimpImporter.ReadFile(
		FullPath.string(),
		aiProcess_MakeLeftHanded |
		aiProcess_FlipUVs |
		aiProcess_FlipWindingOrder |
		aiProcess_Triangulate |
		aiProcess_CalcTangentSpace |
		aiProcess_ValidateDataStructure |
		aiProcess_ImproveCacheLocality |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_GenUVCoords |
		aiProcess_TransformUVCoords |
		aiProcess_FindInstances |
		aiProcess_GenSmoothNormals |
		aiProcess_SortByPType |
		aiProcess_OptimizeMeshes |
		aiProcess_SplitLargeMeshes
	);

	if (AiScene)
	{
		auto& ResourceSys = ResourceSystem::Instance;
		const uint32 MeshNum = AiScene->mNumMeshes;

		_SkyMesh.VtxBufs.resize(MeshNum);
		_SkyMesh.IdxBufs.resize(MeshNum);
		_SkyMesh.VertexCounts.resize(MeshNum);
		_SkyMesh.PrimCount.resize(MeshNum);
		for (uint32 MeshIdx = 0u; MeshIdx < MeshNum; ++MeshIdx)
		{
			std::vector<VertexType> VtxArr{};
			const aiMesh* const CurMesh = AiScene->mMeshes[MeshIdx];
			const uint32 CurMeshVerticesCount = CurMesh->mNumVertices;
			_SkyMesh.VertexCounts[MeshIdx] = CurMeshVerticesCount; 
			VtxArr.resize(CurMeshVerticesCount);
			for (uint32 VtxIdx = 0u; VtxIdx < CurMeshVerticesCount; ++VtxIdx)
			{
				VertexType CurVtx = VertexType::MakeFromAssimpMesh(CurMesh, VtxIdx);
				VtxArr[VtxIdx] = std::move(CurVtx);
			}

			if (AiScene->HasMaterials())
			{
				const uint32 CurMeshMtlIdx = CurMesh->mMaterialIndex;
				const aiMaterial* const CurMeshMtl = AiScene->mMaterials[CurMeshMtlIdx];
				if (CurMeshMtl->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE) > 0)
				{
					aiString SkyTexPath{};
					if (CurMeshMtl->GetTexture
					(aiTextureType::aiTextureType_DIFFUSE, 0, &SkyTexPath, NULL, NULL,
						NULL, NULL, NULL) == AI_SUCCESS)
					{
						const std::wstring LoadFilePathW = ToW(SkyTexPath.C_Str());
						using ResourceType = std::remove_pointer_t<decltype(_SkyMesh.Texture)>;
						_SkyMesh.Texture = ResourceSys->Get<ResourceType>(LoadFilePathW);
						if (nullptr == _SkyMesh.Texture)
						{
							
							D3DXCreateTextureFromFile(Device,
								(FullPath.parent_path() / LoadFilePathW).c_str(), &_SkyMesh.Texture);
							if (_SkyMesh.Texture)
							{
								ResourceSys->Insert< ResourceType>(LoadFilePathW, _SkyMesh.Texture);
							}
						}
					}
				}
			}
			const uint32 VtxBufSize = sizeof(VertexType) * CurMeshVerticesCount;
			
			Device->CreateVertexBuffer(VtxBufSize,D3DUSAGE_WRITEONLY,VertexType::FVF,D3DPOOL_MANAGED,
				&_SkyMesh.VtxBufs[MeshIdx], nullptr); 
			VertexType* CurVtxBufPtr{ nullptr }; 
			_SkyMesh.VtxBufs[MeshIdx]->Lock(0, 0, reinterpret_cast<void**> (&CurVtxBufPtr), NULL);
			std::memcpy(CurVtxBufPtr, VtxArr.data(), VtxBufSize);
			_SkyMesh.VtxBufs[MeshIdx]->Unlock(); 
			
			
			


			std::vector<uint32> IndicesArr{};
			const uint32 CurMeshFaceCount = CurMesh->mNumFaces;
			_SkyMesh.PrimCount[MeshIdx] = CurMeshFaceCount;
			for (uint32 FaceIdx = 0u; FaceIdx < CurMeshFaceCount; ++FaceIdx)
			{
				const aiFace CurFace = CurMesh->mFaces[FaceIdx];
				const uint32 CurIndicesCount = CurFace.mNumIndices;
				for (uint32 IndicesIdx = 0u; IndicesIdx < CurIndicesCount; ++IndicesIdx)
				{
					IndicesArr.push_back(CurFace.mIndices[IndicesIdx]);
				}
			}
			const uint32 IdxBufSize = IndicesArr.size() * sizeof(decltype(IndicesArr)::value_type); 
			Device->CreateIndexBuffer
			(IdxBufSize ,D3DUSAGE_WRITEONLY,D3DFMT_INDEX32,D3DPOOL_MANAGED,&_SkyMesh.IdxBufs[MeshIdx],nullptr);
			uint32* IdxBufPtr{ nullptr }; 
			
			_SkyMesh.IdxBufs[MeshIdx]->Lock(0, 0, reinterpret_cast<void**>   (&IdxBufPtr), NULL);
			std::memcpy(IdxBufPtr, IndicesArr.data(), IdxBufSize);
			_SkyMesh.IdxBufs[MeshIdx]->Unlock();
		};
	}
}

void Engine::Sky::Render(const Vector3& CameraLocation , IDirect3DDevice9* const Device)&
{
	Rotation += Timer::Instance->GetDelta() * RotationAcc;

	ImGui::Begin("Sky");
	static bool bRender{ true };
	ImGui::Checkbox("bRender", &bRender); 
	ImGui::End();
	const Matrix SkyLocation = FMath::WorldMatrix(0.1f, { FMath::ToRadian(90.f),0,Rotation }, CameraLocation);
	if (bRender)
	{
		Device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		Device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		DWORD PrevLightFlag{ false };
		Device->GetRenderState(D3DRS_LIGHTING, &PrevLightFlag);
		Device->SetRenderState(D3DRS_LIGHTING, FALSE);


		Device->SetTransform(D3DTS_WORLD, &SkyLocation);
		Device->SetTexture(0u, _SkyMesh.Texture);
		/*IDirect3DSurface9* DepthBuf{ nullptr };
		Device->GetDepthStencilSurface(&DepthBuf);
		D3DVIEWPORT9 ViewPort{};
		Device->GetViewport(&ViewPort);
		Device->CreateTexture(ViewPort.Width, ViewPort.Height,1u,D3DUSAGE_)*/
		const uint32 SubSetCount = _SkyMesh.VtxBufs.size();
		for (uint32 i = 0; i < SubSetCount; ++i)
		{
			Device->SetIndices(_SkyMesh.IdxBufs[i]);
			Device->SetStreamSource(0u, _SkyMesh.VtxBufs[i], 0u, _SkyMesh.Stride);
			Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, _SkyMesh.VertexCounts[i], 0u, _SkyMesh.PrimCount[i]);
		}
		Device->SetRenderState(D3DRS_LIGHTING, PrevLightFlag);
		Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		Device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		Device->SetRenderState(D3DRS_ZENABLE, TRUE);
		Device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	}
	

	//DepthBuf->Release();
}

