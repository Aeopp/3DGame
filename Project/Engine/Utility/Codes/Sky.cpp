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
						using ResourceType = std::remove_pointer_t<decltype(_SkyMesh.DiffuseTexture)>;
						_SkyMesh.DiffuseTexture = ResourceSys->Get<ResourceType>(LoadFilePathW);
						if (nullptr == _SkyMesh.DiffuseTexture)
						{
							
							D3DXCreateTextureFromFile(Device,
								(FullPath.parent_path() / LoadFilePathW).c_str(), &_SkyMesh.DiffuseTexture);
							if (_SkyMesh.DiffuseTexture)
							{
								ResourceSys->Insert< ResourceType>(LoadFilePathW, _SkyMesh.DiffuseTexture);
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

		const std::wstring VtxDeclName = ToW(typeid(VertexType).name()); 
		_SkyMesh.VtxDecl  = ResourceSys->Get<IDirect3DVertexDeclaration9>(VtxDeclName);
		if ( nullptr == _SkyMesh.VtxDecl )
		{
			_SkyMesh.VtxDecl = VertexType::GetVertexDecl(Device); 
			ResourceSys->Insert<IDirect3DVertexDeclaration9>(VtxDeclName , _SkyMesh.VtxDecl);
		}
	}

	SkyShaderFx.Initialize(L"SkyFx");
}

void Engine::Sky::Render(Engine::Frustum& RefFrustum,
	const Matrix& View, const Matrix& Projection,
	const Vector4& CameraLocation4D,
	IDirect3DDevice9* const Device ,
	IDirect3DTexture9* DepthTexture)&
{
	Rotation += Timer::Instance->GetDelta() * RotationAcc;


	auto Fx = SkyShaderFx.GetHandle();
	const Matrix SkyLocation = FMath::WorldMatrix(0.1f,
		{ FMath::ToRadian(90.f),0,Rotation }, { CameraLocation4D.x,CameraLocation4D.y ,CameraLocation4D.z } );
	if (Fx)
	{
		Fx->SetMatrix("World", &SkyLocation);
		Fx->SetMatrix("View", &View);
		Fx->SetMatrix("Projection", &Projection);
		Device->SetVertexDeclaration(_SkyMesh.VtxDecl);

		uint32 Pass;
		Fx->Begin(&Pass, 0);
		for (uint32 i = 0; i < Pass; ++i)
		{
			Fx->BeginPass(i);
			const uint32 SubSetCount = _SkyMesh.VtxBufs.size();
			for (uint32 i = 0; i < SubSetCount; ++i)
			{
				Fx->SetTexture("DiffuseMap", _SkyMesh.DiffuseTexture);
				Fx->SetTexture("WorldLocationDepth", DepthTexture);
				Fx->CommitChanges();

				Device->SetIndices(_SkyMesh.IdxBufs[i]);
				Device->SetStreamSource(0u, _SkyMesh.VtxBufs[i], 0u, _SkyMesh.Stride);
				Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, _SkyMesh.VertexCounts[i], 0u, _SkyMesh.PrimCount[i]);
			}
			Fx->EndPass();
		}
		Fx->End();
	}
	
	
}

