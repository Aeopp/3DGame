#pragma once
#include "Mesh.h"
#include "Vertexs.hpp"
#include <string>
#include "ExportUtility.hpp"

#include "UtilityGlobal.h"
#include "ResourceSystem.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <optional>
#include <set>
#include "FMath.hpp"
#include <filesystem>
#include <memory>

namespace Engine
{
	class DLL_DECL StaticMesh : public Mesh
	{
	public:
		using Super = Mesh;
	public:
		static const inline Property TypeProperty = Property::Render;
	public:
		StaticMesh() = default;
		StaticMesh(const StaticMesh&) = default;
		StaticMesh(StaticMesh&&)noexcept = default;
		StaticMesh& operator=(const StaticMesh&) = default;
		StaticMesh& operator=(StaticMesh&&)noexcept = default;
	public:
		template<typename VertexType>
		void  Load(IDirect3DDevice9* const Device,
			const std::filesystem::path FilePath,
			const std::filesystem::path FileName,
			const std::wstring ResourceName)&;
		void  Initialize(const std::wstring& ResourceName)&;
		void  Event(class Object* Owner) & override;
		void  Render() & override;
	public:
		std::vector       <MeshElement>                          MeshContainer{};
	};
	
}

template<typename VertexType>
inline void Engine::StaticMesh::Load(
	IDirect3DDevice9* const Device, 
	const std::filesystem::path FilePath, 
	const std::filesystem::path FileName, 
	const std::wstring ResourceName)&
{
	this->Device = Device;
	AiScene = Engine::Global::AssimpImporter.ReadFile(
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

	auto& ResourceSys = RefResourceSys();
	LocalVertexLocations = std::make_shared<std::vector<Vector3>>();

	for (uint32 MeshIdx = 0u; MeshIdx < AiScene->mNumMeshes; ++MeshIdx)
	{
		const std::wstring CurrentResourceName =
			ResourceName + L"_" + std::to_wstring(StaticMeshResourceID++);

		MeshElement CreateMesh{};
		aiMesh* _AiMesh = AiScene->mMeshes[MeshIdx];
		// 버텍스 버퍼.
		std::shared_ptr<std::vector<VertexType>> Verticies = std::make_shared<std::vector<VertexType>>();

		for (uint32 VerticesIdx = 0u; VerticesIdx < _AiMesh->mNumVertices; ++VerticesIdx)
		{
			Verticies->push_back(VertexType::MakeFromAssimpMesh(_AiMesh, VerticesIdx));
			LocalVertexLocations->push_back(FromAssimp(_AiMesh->mVertices[VerticesIdx]));
		}

		const std::wstring MeshVtxBufResourceName =
			L"StaticMesh_VertexBuffer_" + CurrentResourceName;
		IDirect3DVertexBuffer9* _VertexBuffer{ nullptr };
		CreateMesh.VtxBufSize = sizeof(VertexType) * Verticies->size();
		Device->CreateVertexBuffer(CreateMesh.VtxBufSize, D3DUSAGE_WRITEONLY, VertexType::FVF,
			D3DPOOL_MANAGED, &_VertexBuffer, nullptr);
		CreateMesh.VertexBuffer =
			ResourceSys.Insert<IDirect3DVertexBuffer9>(MeshVtxBufResourceName, _VertexBuffer);
		CreateMesh.PrimitiveCount = CreateMesh.FaceCount = _AiMesh->mNumFaces;
		VertexType* VertexBufferPtr{ nullptr };
		CreateMesh.VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBufferPtr), NULL);
		std::memcpy(VertexBufferPtr, Verticies->data(), CreateMesh.VtxBufSize);
		CreateMesh.VertexBuffer->Unlock();
		CreateMesh.VtxCount = Verticies->size();
		CreateMesh.Stride = sizeof(VertexType);
		CreateMesh.FVF = VertexType::FVF;
		// 인덱스 버퍼.
		std::vector<uint32> Indicies{};
		for (uint32 FaceIdx = 0u; FaceIdx < _AiMesh->mNumFaces; ++FaceIdx)
		{
			const aiFace CurrentFace = _AiMesh->mFaces[FaceIdx];
			for (uint32 Idx = 0u; Idx < CurrentFace.mNumIndices; ++Idx)
			{
				Indicies.push_back(CurrentFace.mIndices[Idx]);
			}
		}
		const std::wstring MeshIdxBufResourceName =
			L"StaticMesh_IndexBuffer_" + CurrentResourceName;
		IDirect3DIndexBuffer9* _IndexBuffer{ nullptr };
		const uint32 IdxBufSize = sizeof(uint32) * Indicies.size();
		Device->CreateIndexBuffer(IdxBufSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX32,
			D3DPOOL_MANAGED, &_IndexBuffer, nullptr);
		CreateMesh.IndexBuffer = ResourceSys.Insert<IDirect3DIndexBuffer9>(MeshIdxBufResourceName,
			_IndexBuffer);
		uint32* IndexBufferPtr{ nullptr };
		CreateMesh.IndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&IndexBufferPtr), NULL);
		std::memcpy(IndexBufferPtr, Indicies.data(), IdxBufSize);
		CreateMesh.IndexBuffer->Unlock();
		// 머테리얼.
		aiMaterial* AiMaterial = AiScene->mMaterials[_AiMesh->mMaterialIndex];
		if (AiMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType_DIFFUSE, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == aiReturn::aiReturn_SUCCESS)
			{
				const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();
				const std::wstring TexResourceName = L"StaticMesh_DiffuseTexture_" + CurrentResourceName;

				ResourceSys.Emplace<IDirect3DTexture9>
					(TexResourceName,
						D3DXCreateTextureFromFile, Device, TexFileFullPath.c_str(), &CreateMesh.DiffuseTexture);
			}
		}
		if (AiMaterial->GetTextureCount(aiTextureType::aiTextureType_NORMALS) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == aiReturn::aiReturn_SUCCESS)
			{
				const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();
				const std::wstring TexResourceName = L"StaticMesh_NormalTexture_" + CurrentResourceName;

				ResourceSys.Emplace<IDirect3DTexture9>
					(TexResourceName,
						D3DXCreateTextureFromFile, Device, TexFileFullPath.c_str(), &CreateMesh.NormalTexture);
			}
		}
		if (AiMaterial->GetTextureCount(aiTextureType::aiTextureType_SPECULAR) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType::aiTextureType_SPECULAR, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == aiReturn::aiReturn_SUCCESS)
			{
				const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();
				const std::wstring TexResourceName = L"StaticMesh_SpecularTexture_" + CurrentResourceName;

				ResourceSys.Emplace<IDirect3DTexture9>
					(TexResourceName,
						D3DXCreateTextureFromFile, Device, TexFileFullPath.c_str(), &CreateMesh.SpecularTexture);
			}
		}
		MeshContainer.push_back(CreateMesh);
	}
}