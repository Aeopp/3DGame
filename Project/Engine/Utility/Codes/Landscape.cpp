#include "Landscape.h"
#include "Transform.h"
#include "imgui.h"
#include "FMath.hpp"
#include "AssimpHelper.h"
#include "UtilityGlobal.h"
#include "ResourceSystem.h"
#include "Vertexs.hpp"

// TODO :: 노말매핑 사용시 버텍스 타입 변경해야 하며. FVF 변경도 고려 해야함 . 
// 또한 템플릿 코드로 변경시 리소스 시스템 접근 방식 변경 해야할 수도 있음 . 
void Engine::Landscape::Initialize(
	IDirect3DDevice9* const Device,
	const Matrix& MapWorld,
	const std::filesystem::path FilePath,
	const std::filesystem::path FileName)&
{
	 //               TODO ::  노말 매핑 사용할시 변경 바람 . 
	using VertexType = Vertex::LocationNormalUV2D;
	this->Device = Device;
	World = MapWorld;
	auto AiScene = Engine::Global::AssimpImporter.ReadFile(
		(FilePath / FileName).string(),
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

	static uint32 StaticMeshResourceID = 0u;
	auto& ResourceSys = ResourceSystem::Instance;
	Meshes.resize(AiScene->mNumMeshes);
	for (uint32 MeshIdx = 0u; MeshIdx < AiScene->mNumMeshes; ++MeshIdx)
	{
		std::vector<VertexType> Vertexs;
		const std::wstring ResourceIDStr = std::to_wstring(StaticMeshResourceID++); 
		aiMesh* AiMesh = AiScene->mMeshes[MeshIdx];
		
		Meshes[MeshIdx].VtxCount = AiMesh->mNumVertices; 
		Meshes[MeshIdx].Stride = sizeof(VertexType);
		for (uint32 VerticesIdx = 0u; VerticesIdx < Meshes[MeshIdx].VtxCount; ++VerticesIdx)
		{
			Vertexs.emplace_back(VertexType::MakeFromAssimpMesh(AiMesh, VerticesIdx));
			LocalVertexLocations.push_back(FromAssimp(AiMesh->mVertices[VerticesIdx])); 
		};

		
		const uint32 VtxBufsize = Meshes[MeshIdx].VtxCount * Meshes[MeshIdx].Stride;
		Device->CreateVertexBuffer(VtxBufsize, D3DUSAGE_WRITEONLY, VertexType::FVF,
			D3DPOOL_MANAGED, &Meshes[MeshIdx].VtxBuf, nullptr);

		ResourceSys->Insert<IDirect3DVertexBuffer9>(L"VertexBuffer_Landscape_" + ResourceIDStr, Meshes[MeshIdx].VtxBuf);
		Meshes[MeshIdx].PrimitiveCount = AiMesh->mNumFaces;
		VertexType* VtxBufPtr{ nullptr }; 
		Meshes[MeshIdx].VtxBuf->Lock(0u, 0u, reinterpret_cast<void**>( &VtxBufPtr), NULL);
		std::memcpy(VtxBufPtr, Vertexs.data(), VtxBufsize);
		Meshes[MeshIdx].VtxBuf->Unlock();
		Meshes[MeshIdx].FVF = VertexType::FVF;
		
		
		// 인덱스 버퍼 파싱. 
		std::vector<uint32> Indicies{};
		for (uint32 FaceIdx = 0u; FaceIdx < AiMesh->mNumFaces; ++FaceIdx)
		{
			const aiFace  CurrentFace = AiMesh->mFaces[FaceIdx]; 
			for (uint32 Idx = 0u; Idx < CurrentFace.mNumIndices; ++Idx)
			{
				Indicies.push_back(CurrentFace.mIndices[Idx]);
			}; 
		};
		const uint32 IdxBufSize = sizeof(uint32) * Indicies.size();
		Device->CreateIndexBuffer(IdxBufSize,D3DUSAGE_WRITEONLY,D3DFMT_INDEX32,
			D3DPOOL_MANAGED, &Meshes[MeshIdx].IdxBuf, nullptr); 
		ResourceSys->Insert<IDirect3DIndexBuffer9>(L"IndexBuffer_Landscape_" + ResourceIDStr, Meshes[MeshIdx].IdxBuf);
		uint32* IdxBufPtr{ nullptr }; 
		Meshes[MeshIdx].IdxBuf->Lock(0, 0, reinterpret_cast<void**>( &IdxBufPtr), NULL);
		std::memcpy(IdxBufPtr, Indicies.data(), IdxBufSize);
		Meshes[MeshIdx].IdxBuf->Unlock(); 

		// 머테리얼 파싱 . 
		aiMaterial* AiMaterial = AiScene->mMaterials[AiMesh->mMaterialIndex];

		if (AiMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType_DIFFUSE, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == aiReturn::aiReturn_SUCCESS)
			{
				const std::string TextureName = AiFileName.C_Str();
				std::wstring TextureNameW;
				TextureNameW.assign(std::begin(TextureName), std::end(TextureName)); 
				Meshes[MeshIdx].DiffuseMap = 
					ResourceSys->Get<IDirect3DTexture9>(TextureNameW);

				if (Meshes[MeshIdx].DiffuseMap == nullptr)
				{
					const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();

					Meshes[MeshIdx].DiffuseMap = ResourceSys->Emplace<IDirect3DTexture9>
						(TextureNameW,
							D3DXCreateTextureFromFile, Device, TexFileFullPath.c_str(), 
							&Meshes[MeshIdx].DiffuseMap);
				}
			}
		}
		
		if (AiMaterial->GetTextureCount(aiTextureType::aiTextureType_SPECULAR) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType::aiTextureType_SPECULAR, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == aiReturn::aiReturn_SUCCESS)
			{
				const std::string TextureName = AiFileName.C_Str();
				std::wstring TextureNameW;
				TextureNameW.assign(std::begin(TextureName), std::end(TextureName));
				Meshes[MeshIdx].SpecularMap =
					ResourceSys->Get<IDirect3DTexture9>(TextureNameW);

				if (Meshes[MeshIdx].SpecularMap == nullptr)
				{
					const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();

					Meshes[MeshIdx].SpecularMap = ResourceSys->Emplace<IDirect3DTexture9>
						(TextureNameW,
							D3DXCreateTextureFromFile, Device, TexFileFullPath.c_str(),
							&Meshes[MeshIdx].SpecularMap);
				}
			}
		}

		if (AiMaterial->GetTextureCount(aiTextureType::aiTextureType_NORMALS) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == aiReturn::aiReturn_SUCCESS)
			{
				const std::string TextureName = AiFileName.C_Str();
				std::wstring TextureNameW;
				TextureNameW.assign(std::begin(TextureName), std::end(TextureName));
				Meshes[MeshIdx].NormalMap =
					ResourceSys->Get<IDirect3DTexture9>(TextureNameW);

				if (Meshes[MeshIdx].NormalMap == nullptr)
				{
					const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();

					Meshes[MeshIdx].NormalMap = ResourceSys->Emplace<IDirect3DTexture9>
						(TextureNameW,
							D3DXCreateTextureFromFile, Device, TexFileFullPath.c_str(),
							&Meshes[MeshIdx].NormalMap);
				}
			}
		}
	};
}

void Engine::Landscape::Render(Engine::Frustum& RefFrustum)&
{
	if (nullptr == Device)
		return;

	Device->SetTransform(D3DTS_WORLD, &World); 

	for (auto& CurMesh : Meshes)
	{
		Device->SetStreamSource(0, CurMesh.VtxBuf, 0, CurMesh.Stride);
		Device->SetFVF(CurMesh.FVF);
		Device->SetTexture(0, CurMesh.DiffuseMap);
		Device->SetIndices(CurMesh.IdxBuf);
		Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0u, CurMesh.VtxCount,
			0u, CurMesh.PrimitiveCount);
	}

	// RefFrustum.IsIn()
}
