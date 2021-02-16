#include "Landscape.h"
#include "Transform.h"
#include "imgui.h"
#include "FMath.hpp"
#include "AssimpHelper.h"
#include "UtilityGlobal.h"
#include "ResourceSystem.h"
#include "Vertexs.hpp"
#include "ShaderManager.h"
#include "Renderer.h"

#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/reader.h> 
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <ostream>
#include "StringHelper.h"

static uint32 StaticMeshResourceID = 0u;
static Engine::Landscape::DecoInformation* PickDecoInstancePtr{ nullptr };

void Engine::Landscape::DecoratorLoad(
	const std::filesystem::path& LoadPath,
	const std::filesystem::path& LoadFileName)&
{
	using VertexType = Vertex::LocationTangentUV2D;

	auto AiScene = Engine::Global::AssimpImporter.ReadFile(
		(LoadPath / LoadFileName).string(),
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

	Decorator LoadDecorator{};

	auto& ResourceSys = ResourceSystem::Instance;

	LoadDecorator.Meshes.resize(AiScene->mNumMeshes);


	std::vector<Vector3> DecoLocalVertexLocations;

	for (uint32 MeshIdx = 0u; MeshIdx < AiScene->mNumMeshes; ++MeshIdx)
	{
		std::vector<VertexType> Vertexs;
		const std::wstring ResourceIDStr = std::to_wstring(StaticMeshResourceID++);
		aiMesh* AiMesh = AiScene->mMeshes[MeshIdx];



		LoadDecorator.Meshes[MeshIdx].Name = AiMesh->mName.C_Str();

		LoadDecorator.Meshes[MeshIdx].VtxCount = AiMesh->mNumVertices;
		LoadDecorator.Meshes[MeshIdx].Stride = sizeof(VertexType);
		for (uint32 VerticesIdx = 0u; VerticesIdx < LoadDecorator.Meshes[MeshIdx].VtxCount; ++VerticesIdx)
		{
			Vertexs.emplace_back(VertexType::MakeFromAssimpMesh(AiMesh, VerticesIdx));
			const Vector3 LocalVertexLocation = FromAssimp(AiMesh->mVertices[VerticesIdx]); 
			DecoLocalVertexLocations.push_back(LocalVertexLocation);
		};

		D3DXComputeBoundingSphere(reinterpret_cast<const Vector3*>(Vertexs.data()),
			Vertexs.size(), sizeof(VertexType),
			&LoadDecorator.Meshes[MeshIdx].BoundingSphere.Center,
			&LoadDecorator.Meshes[MeshIdx].BoundingSphere.Radius
		);


		const uint32 VtxBufsize = LoadDecorator.Meshes[MeshIdx].VtxCount * LoadDecorator.Meshes[MeshIdx].Stride;
		Device->CreateVertexBuffer(VtxBufsize, D3DUSAGE_WRITEONLY, 0u,
			D3DPOOL_MANAGED, &LoadDecorator.Meshes[MeshIdx].VtxBuf, nullptr);

		ResourceSys->Insert<IDirect3DVertexBuffer9>(L"VertexBuffer_Decorator_" + ResourceIDStr, LoadDecorator.Meshes[MeshIdx].VtxBuf);
		LoadDecorator.Meshes[MeshIdx].PrimitiveCount = AiMesh->mNumFaces;
		VertexType* VtxBufPtr{ nullptr };
		LoadDecorator.Meshes[MeshIdx].VtxBuf->Lock(0u, 0u, reinterpret_cast<void**>(&VtxBufPtr), NULL);
		std::memcpy(VtxBufPtr, Vertexs.data(), VtxBufsize);
		LoadDecorator.Meshes[MeshIdx].VtxBuf->Unlock();
		LoadDecorator.Meshes[MeshIdx].FVF = 0u;


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
		Device->CreateIndexBuffer(IdxBufSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX32,
			D3DPOOL_MANAGED, &LoadDecorator.Meshes[MeshIdx].IdxBuf, nullptr);
		ResourceSys->Insert<IDirect3DIndexBuffer9>(L"IndexBuffer_Decorator_" + ResourceIDStr, LoadDecorator.Meshes[MeshIdx].IdxBuf);
		uint32* IdxBufPtr{ nullptr };
		LoadDecorator.Meshes[MeshIdx].IdxBuf->Lock(0, 0, reinterpret_cast<void**>(&IdxBufPtr), NULL);
		std::memcpy(IdxBufPtr, Indicies.data(), IdxBufSize);
		LoadDecorator.Meshes[MeshIdx].IdxBuf->Unlock();

		// 머테리얼 파싱 . 
		aiMaterial* AiMaterial = AiScene->mMaterials[AiMesh->mMaterialIndex];

		if (AiMaterial->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType_DIFFUSE, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == aiReturn::aiReturn_SUCCESS)
			{
				const std::string TextureName = AiFileName.C_Str();
				std::wstring TextureNameW;
				TextureNameW.assign(std::begin(TextureName), std::end(TextureName));
				LoadDecorator.Meshes[MeshIdx].DiffuseMap =
					ResourceSys->Get<IDirect3DTexture9>(TextureNameW);

				if (LoadDecorator.Meshes[MeshIdx].DiffuseMap == nullptr)
				{
					const std::filesystem::path TexFileFullPath = LoadPath / AiFileName.C_Str();
					D3DXCreateTextureFromFile(Device, TexFileFullPath.c_str(), &LoadDecorator.Meshes[MeshIdx].DiffuseMap);

					LoadDecorator.Meshes[MeshIdx].DiffuseMap =
						ResourceSys->Insert<IDirect3DTexture9>(TextureNameW, LoadDecorator.Meshes[MeshIdx].DiffuseMap);
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
				LoadDecorator.Meshes[MeshIdx].bCavity = 1;

				const std::string TextureName = AiFileName.C_Str();
				std::wstring TextureNameW;
				TextureNameW.assign(std::begin(TextureName), std::end(TextureName));
				LoadDecorator.Meshes[MeshIdx].CavityMap =
					ResourceSys->Get<IDirect3DTexture9>(TextureNameW);

				if (LoadDecorator.Meshes[MeshIdx].CavityMap == nullptr)
				{
					const std::filesystem::path TexFileFullPath = LoadPath / AiFileName.C_Str();
					D3DXCreateTextureFromFile(Device, TexFileFullPath.c_str(), &LoadDecorator.Meshes[MeshIdx].CavityMap);
					LoadDecorator.Meshes[MeshIdx].CavityMap =
						ResourceSys->Insert<IDirect3DTexture9>(TextureNameW, LoadDecorator.Meshes[MeshIdx].CavityMap);
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
				LoadDecorator.Meshes[MeshIdx].NormalMap =
					ResourceSys->Get<IDirect3DTexture9>(TextureNameW);

				if (LoadDecorator.Meshes[MeshIdx].NormalMap == nullptr)
				{
					const std::filesystem::path TexFileFullPath = LoadPath / AiFileName.C_Str();
					D3DXCreateTextureFromFile(Device, TexFileFullPath.c_str(), &LoadDecorator.Meshes[MeshIdx].NormalMap);
					LoadDecorator.Meshes[MeshIdx].NormalMap =
						ResourceSys->Insert<IDirect3DTexture9>(TextureNameW, LoadDecorator.Meshes[MeshIdx].NormalMap);
				}
			}
		}

		if (AiMaterial->GetTextureCount(aiTextureType::aiTextureType_EMISSIVE) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType::aiTextureType_EMISSIVE, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == aiReturn::aiReturn_SUCCESS)
			{
				const std::string TextureName = AiFileName.C_Str();
				std::wstring TextureNameW;
				TextureNameW.assign(std::begin(TextureName), std::end(TextureName));
				LoadDecorator.Meshes[MeshIdx].EmissiveMap =
					ResourceSys->Get<IDirect3DTexture9>(TextureNameW);

				if (LoadDecorator.Meshes[MeshIdx].EmissiveMap == nullptr)
				{
					const std::filesystem::path TexFileFullPath = LoadPath / AiFileName.C_Str();
					D3DXCreateTextureFromFile(Device, TexFileFullPath.c_str(), &LoadDecorator.Meshes[MeshIdx].EmissiveMap);
					LoadDecorator.Meshes[MeshIdx].EmissiveMap = ResourceSys->Insert<IDirect3DTexture9>(TextureNameW, LoadDecorator.Meshes[MeshIdx].EmissiveMap);
				}
			}
		}
	};

	DecoratorContainer.insert({ LoadFileName,LoadDecorator } );
}

std::weak_ptr<typename Engine::Landscape::DecoInformation>  Engine::Landscape::PushDecorator(
	const std::wstring DecoratorKey, 
	const Vector3& Scale,
	const Vector3& Rotation, 
	const Vector3& Location,
	const bool bLandscapePolygonInclude)&
{
	auto iter = DecoratorContainer.find(DecoratorKey);

	if (iter != std::end(DecoratorContainer))
	{
		auto& [Key, Deco] = *iter;
		Engine::Landscape::DecoInformation DecoInfoInstance{};

		DecoInfoInstance.Scale = Scale;
		DecoInfoInstance.Rotation = Rotation;
		DecoInfoInstance.Location = Location;

		if (bLandscapePolygonInclude)
		{
			const Matrix InstanceWorld = FMath::WorldMatrix(
				DecoInfoInstance.Scale,
				DecoInfoInstance.Rotation,
				DecoInfoInstance.Location
			);  

			for (const auto& CurDecoMesh : iter->second.Meshes)
			{
				byte* VtxBufPtr{ nullptr }; 
				CurDecoMesh.VtxBuf->Lock(0u, 0u, reinterpret_cast<void**>(&VtxBufPtr), NULL);
				for (uint32 i = 0; i < CurDecoMesh.VtxCount; i+=3)
				{
					std::array<Vector3, 3u> WorldPoints{}; 

					for (uint32 j = 0; j < WorldPoints.size(); ++j)
					{
						WorldPoints[j]  =
							*reinterpret_cast<Vector3*>(VtxBufPtr + ((i + j) * CurDecoMesh.Stride)); 
						WorldPoints[j] = FMath::Mul(WorldPoints[j], InstanceWorld);

					};

					WorldPlanes.push_back(PlaneInfo::Make(WorldPoints));
				}
				CurDecoMesh.VtxBuf->Unlock();
			}
		}

		return Deco.Instances.emplace_back(
			std::make_shared<Engine::Landscape::DecoInformation>(DecoInfoInstance));
	}

	return {};
}

typename Engine::Landscape::Decorator* Engine::Landscape::GetDecorator(const std::wstring DecoratorKey)&
{
	if (auto iter = DecoratorContainer.find(DecoratorKey);
		iter != std::end(DecoratorContainer))
	{
		return &(iter->second); 
	}

	return {};
}

std::weak_ptr<typename Engine::Landscape::DecoInformation> 
	Engine::Landscape::PickDecoInstance(const Ray WorldRay)&
{
	for (auto& [Key , CurDeco ] : DecoratorContainer)
	{
		for (const auto& CurDecoMesh : CurDeco.Meshes)
		{
			const Sphere CurMeshLocalSphere = CurDecoMesh.BoundingSphere;

			for (auto& CurInstance : CurDeco.Instances)
			{
				Sphere CurMeshInstanceWorldSphere{}; 

				const Matrix CurInstanceWorld = 
					FMath::WorldMatrix(CurInstance->Scale, CurInstance->Rotation, CurInstance->Location);

				CurMeshInstanceWorldSphere.Center = FMath::Mul(CurMeshLocalSphere.Center, CurInstanceWorld);

				CurMeshInstanceWorldSphere.Radius =
					CurMeshLocalSphere.Radius * FMath::MaxScala(CurInstance->Scale); 

				float t0, t1;
				Vector3 IntersectPt{}; 

				if (FMath::IsRayToSphere(WorldRay, CurMeshInstanceWorldSphere, t0, t1, IntersectPt))
				{
					// 구체피킹 성공하였으니 다음 스텝은 폴리곤과의 오버랩 검사.
					std::vector<PlaneInfo> Polygons; 
					byte* VtxBufPtr{ nullptr }; 
					CurDecoMesh.VtxBuf->Lock(0u, 0u, reinterpret_cast<void**>(&VtxBufPtr), NULL);
					for (uint32 i = 0; i < CurDecoMesh.VtxCount; i+=3)
					{
						std::array<Vector3, 3u> Points{};
						for (uint32 j = 0; j < Points.size(); ++j)
						{
							Points[j] =
								*reinterpret_cast<const Vector3*>((VtxBufPtr + ((i + j) * CurDecoMesh.Stride)));

							Points[j] = FMath::Mul(Points[j] , CurInstanceWorld);
						}

						Polygons.push_back(PlaneInfo::Make(Points));
					}
					CurDecoMesh.VtxBuf->Unlock(); 

					for (const auto& TargetPolygon : Polygons)
					{
						float t;
						Vector3 IntersectPt{}; 
						if (FMath::IsTriangleToRay(TargetPolygon, WorldRay, t, IntersectPt))
						{
							PickDecoInstancePtr = CurInstance.get();
							return CurInstance;
						}
					}
				}
			}
		}
	}
	
	return {};
}



// TODO :: 노말매핑 사용시 버텍스 타입 변경해야 하며. FVF 변경도 고려 해야함 . 
// 또한 템플릿 코드로 변경시 리소스 시스템 접근 방식 변경 해야할 수도 있음 . 
void Engine::Landscape::Initialize(
	IDirect3DDevice9* const Device,
	const Vector3 Scale,
	const Vector3 Rotation,
	const Vector3 Location,
	const std::filesystem::path FilePath,
	const std::filesystem::path FileName)&
{
	 //               TODO ::  노말 매핑 사용할시 변경 바람 . 
	using VertexType = Vertex::LocationTangentUV2D;
	this->Device = Device;
	this->Scale = Scale;
	this->Rotation = Rotation;
	this->Location = Location;

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

	const Matrix MapWorld = FMath::WorldMatrix(this->Scale,Rotation, Location);

	auto& ResourceSys = ResourceSystem::Instance;

	Meshes.resize(AiScene->mNumMeshes);
	std::vector<Vector3>   WorldVertexLocation{};
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
			WorldVertexLocation.push_back(
				FMath::Mul(FromAssimp(AiMesh->mVertices[VerticesIdx]), MapWorld) );
		};

		
		const uint32 VtxBufsize = Meshes[MeshIdx].VtxCount * Meshes[MeshIdx].Stride;
		Device->CreateVertexBuffer(VtxBufsize, D3DUSAGE_WRITEONLY, 0u,
			D3DPOOL_MANAGED, &Meshes[MeshIdx].VtxBuf, nullptr);

		ResourceSys->Insert<IDirect3DVertexBuffer9>(L"VertexBuffer_Landscape_" + ResourceIDStr, Meshes[MeshIdx].VtxBuf);
		Meshes[MeshIdx].PrimitiveCount = AiMesh->mNumFaces;
		VertexType* VtxBufPtr{ nullptr }; 
		Meshes[MeshIdx].VtxBuf->Lock(0u, 0u, reinterpret_cast<void**>( &VtxBufPtr), NULL);
		std::memcpy(VtxBufPtr, Vertexs.data(), VtxBufsize);
		Meshes[MeshIdx].VtxBuf->Unlock();
		Meshes[MeshIdx].FVF = 0u;
		
		
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

		if (AiMaterial->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE) > 0)
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
					D3DXCreateTextureFromFile(Device, TexFileFullPath.c_str(), &Meshes[MeshIdx].DiffuseMap);

					Meshes[MeshIdx].DiffuseMap=
						ResourceSys->Insert<IDirect3DTexture9>(TextureNameW,Meshes[MeshIdx].DiffuseMap);
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
				Meshes[MeshIdx].bCavity = 1;

				const std::string TextureName = AiFileName.C_Str();
				std::wstring TextureNameW;
				TextureNameW.assign(std::begin(TextureName), std::end(TextureName));
				Meshes[MeshIdx].CavityMap =
					ResourceSys->Get<IDirect3DTexture9>(TextureNameW);

				if (Meshes[MeshIdx].CavityMap == nullptr)
				{
					const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();
					D3DXCreateTextureFromFile(Device, TexFileFullPath.c_str(), &Meshes[MeshIdx].CavityMap);
					Meshes[MeshIdx].CavityMap=
						ResourceSys->Insert<IDirect3DTexture9>(TextureNameW, Meshes[MeshIdx].CavityMap);
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
					D3DXCreateTextureFromFile(Device, TexFileFullPath.c_str(), &Meshes[MeshIdx].NormalMap);
					Meshes[MeshIdx].NormalMap=	
						ResourceSys->Insert<IDirect3DTexture9>(TextureNameW, Meshes[MeshIdx].NormalMap);
				}
			}
		}

		if (AiMaterial->GetTextureCount(aiTextureType::aiTextureType_EMISSIVE) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType::aiTextureType_EMISSIVE, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == aiReturn::aiReturn_SUCCESS)
			{
				const std::string TextureName = AiFileName.C_Str();
				std::wstring TextureNameW;
				TextureNameW.assign(std::begin(TextureName), std::end(TextureName));
				Meshes[MeshIdx].EmissiveMap=
					ResourceSys->Get<IDirect3DTexture9>(TextureNameW);

				if (Meshes[MeshIdx].EmissiveMap == nullptr)
				{
					const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();
					D3DXCreateTextureFromFile(Device, TexFileFullPath.c_str(), &Meshes[MeshIdx].EmissiveMap);
					Meshes[MeshIdx].EmissiveMap=ResourceSys->Insert<IDirect3DTexture9>(TextureNameW, Meshes[MeshIdx].EmissiveMap);
				}
			}
		}
	};

	for (uint32 i = 0; i < WorldVertexLocation.size(); i += 3u)
	{
		WorldPlanes.push_back(
			PlaneInfo::Make({ WorldVertexLocation[i]  ,
				WorldVertexLocation[i + 1]  , 
				WorldVertexLocation[i + 2] })
		); 
	}

	std::string  VtxTypeName = typeid(VertexType).name();
	std::wstring VtxTypeWName;
	VtxTypeWName.assign(std::begin(VtxTypeName), std::end(VtxTypeName));
	VtxDecl  = ResourceSys->Get<IDirect3DVertexDeclaration9>(VtxTypeWName);

	if (!VtxDecl)
	{
		VtxDecl = VertexType::GetVertexDecl(Device);
		ResourceSys->Insert<IDirect3DVertexDeclaration9>(VtxTypeWName , VtxDecl);
	}

	_ShaderFx.Initialize(L"LandscapeFx");
	_ShaderFxNonCavity.Initialize(L"LandscapeFxNonCavity");
}


void Engine::Landscape::Render(Engine::Frustum& RefFrustum,
	const Matrix& View, const Matrix& Projection,
	const Vector3& CameraLocation)&
{
	for (auto& [DecoKey, CurDeco] : DecoratorContainer)
	{
		CurDeco.Instances.erase(std::remove_if(std::begin(CurDeco.Instances), std::end(CurDeco.Instances),
			[](auto& TargetInstance) {
				return TargetInstance->bPendingKill;
			}) ,std::end(CurDeco.Instances) );
		
	}
	if (nullptr == Device)
		return;

	auto& Renderer = *Engine::Renderer::Instance;
	const Matrix MapWorld = FMath::WorldMatrix(Scale, Rotation, Location);

	auto Fx = _ShaderFx.GetHandle();
	uint32 PassNum = 0u;
	Fx->Begin(&PassNum, 0);
	Fx->SetMatrix("World", &MapWorld);
	Fx->SetMatrix("View", &View);
	Fx->SetMatrix("Projection", &Projection);
	Fx->SetVector("LightDirection", &Renderer.LightDirection);
	Fx->SetVector("LightColor", &Renderer.LightColor);
	const Vector4 CameraLocation4D = Vector4{ CameraLocation.x,CameraLocation.y,CameraLocation.z ,1.f };
	Fx->SetVector("CameraLocation", &CameraLocation4D);
	for (uint32 i = 0; i < PassNum; ++i)
	{
		Fx->BeginPass(i);
		for (auto& CurMesh : Meshes)
		{
			Fx->SetVector("RimAmtColor", &CurMesh.RimAmtColor);
			
			Fx->SetFloat("RimOuterWidth", CurMesh.RimOuterWidth);
			Fx->SetFloat("RimInnerWidth", CurMesh.RimInnerWidth);
			Fx->SetVector("AmbientColor", &CurMesh.AmbientColor);
			Fx->SetFloat("Power", CurMesh.Power);
			Fx->SetFloat("SpecularIntencity", CurMesh.SpecularIntencity);
			Fx->SetFloat("Contract", CurMesh.Contract); 
			Device->SetVertexDeclaration(VtxDecl);
			Device->SetStreamSource(0, CurMesh.VtxBuf, 0, CurMesh.Stride);
			Device->SetIndices(CurMesh.IdxBuf);

			Fx->SetTexture("DiffuseMap", CurMesh.DiffuseMap);
			Fx->SetTexture("NormalMap", CurMesh.NormalMap);
			Fx->SetInt("bCavity", CurMesh.bCavity);

			if(CurMesh.bCavity==1)
				Fx->SetTexture("CavityMap", CurMesh.CavityMap);

			Fx->SetTexture("EmissiveMap", CurMesh.EmissiveMap);
			Fx->CommitChanges();
			Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, CurMesh.VtxCount,
				0u, CurMesh.PrimitiveCount);
		}
		Fx->EndPass();
	}
	Fx->End();

	if(Engine::Global::bDebugMode)
		ImGui::Begin("Frustum Culling Log");

	for (const auto& [DecoKey, CurDeco] : DecoratorContainer)
	{
		for (const auto& CurDecoInstance : CurDeco.Instances)
		{
			const Vector3 DecoTfmScale = CurDecoInstance->Scale;
			const Vector3 DecoTfmLocation = CurDecoInstance->Location;
			const Vector3  DecoTfmRotation = CurDecoInstance->Rotation;

			const Matrix DecoWorld = 
				FMath::WorldMatrix(
					DecoTfmScale, 
					DecoTfmRotation, DecoTfmLocation);

			uint32 PassNum = 0u;
			Fx->Begin(&PassNum, 0);
			Fx->SetMatrix("World", &DecoWorld);
			Fx->SetMatrix("View", &View);
			Fx->SetMatrix("Projection", &Projection);
			Fx->SetVector("CameraLocation", &CameraLocation4D);

		

			for (uint32 i = 0; i < PassNum; ++i)
			{
				Fx->BeginPass(i);
				for (auto& CurMesh : CurDeco.Meshes)
				{
					Sphere CurDecoMeshBoundingSphere;
					CurDecoMeshBoundingSphere.Center = FMath::Mul(CurMesh.BoundingSphere.Center, DecoWorld);
					CurDecoMeshBoundingSphere.Radius = (CurMesh.BoundingSphere.Radius * FMath::MaxScala(DecoTfmScale));
					const bool bRender = RefFrustum.IsIn(CurDecoMeshBoundingSphere);

					if (Engine::Global::bDebugMode)
					{
						if (bRender)
						{
							ImGui::TextColored(ImVec4{ 1.f,107.f / 255.f,181.f / 255.f,0.5f }, "Draw : %s", CurMesh.Name.c_str());
						}
						else
						{
							ImGui::TextColored(ImVec4{ 158.f / 255.f,158.f/ 255.f,255.f,0.5f }, "Culling : %s", CurMesh.Name.c_str());
						}
					}
					
					if (bRender)
					{
						Device->SetVertexDeclaration(VtxDecl);
						Device->SetStreamSource(0, CurMesh.VtxBuf, 0, CurMesh.Stride);
						Device->SetIndices(CurMesh.IdxBuf);
						const Vector4 RimAmtColor{ 1.f,1.f,1.f, 1.f };
						Fx->SetVector("RimAmtColor", &CurMesh.RimAmtColor);
						Fx->SetFloat("RimOuterWidth", CurMesh.RimOuterWidth);
						Fx->SetFloat("RimInnerWidth", CurMesh.RimInnerWidth);
						Fx->SetFloat("Power", CurMesh.Power);
						Fx->SetFloat("SpecularIntencity", CurMesh.SpecularIntencity);
						Fx->SetFloat("Contract",CurMesh.Contract);
						if (Engine::Global::bDebugMode 
							&& (PickDecoInstancePtr == CurDecoInstance.get()))
						{
							const Vector4 PickAccentAmbient{25.f,0.f,255.f,1.f};
							Fx->SetVector("AmbientColor", &PickAccentAmbient);
						}
						else
						{
							Fx->SetVector("AmbientColor", &CurMesh.AmbientColor);
						}
						Fx->SetTexture("DiffuseMap", CurMesh.DiffuseMap);
						Fx->SetTexture("NormalMap", CurMesh.NormalMap);
						Fx->SetInt("bCavity", CurMesh.bCavity);
						
						if(CurMesh.bCavity==1)
							Fx->SetTexture("CavityMap", CurMesh.CavityMap);

						Fx->SetTexture("EmissiveMap", CurMesh.EmissiveMap);
						Fx->CommitChanges();
						Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, CurMesh.VtxCount,
							0u, CurMesh.PrimitiveCount);
					}
				}
				Fx->EndPass();
			}
			Fx->End();
		}
	}

	if (Engine::Global::bDebugMode)
		ImGui::End();
	
	if (Engine::Global::bDebugMode && bDecoratorSphereMeshRender)
	{
		auto& _ResourceSys = *ResourceSystem::Instance;
		ID3DXMesh* DebugSphere = _ResourceSys.Get<ID3DXMesh>(L"SphereMesh");
		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

		for (const auto& [DecoKey, CurDeco] : DecoratorContainer)
		{
			for (const auto& CurDecoTransform : CurDeco.Instances)
			{
				const Vector3 DecoTfmScale    = CurDecoTransform->Scale;
				const Vector3 DecoTfmLocation = CurDecoTransform->Location;
				const Vector3  DecoTfmRotation = CurDecoTransform->Rotation;

				for (const auto& _Mesh :CurDeco.Meshes)
				{
					const Matrix SphereLocalMatrix = FMath::WorldMatrix(
						_Mesh.BoundingSphere.Radius 
						, { 0,0,0 },
						_Mesh.BoundingSphere.Center);

					const Matrix DecoWorld=SphereLocalMatrix* FMath::WorldMatrix(
						FMath::MaxScala( DecoTfmScale )  ,
						DecoTfmRotation, DecoTfmLocation);

					Device->SetTransform(D3DTS_WORLD, &DecoWorld);
					DebugSphere->DrawSubset(0u);
				}
			}
		}

		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}
}


void Engine::Landscape::Save(const std::filesystem::path& SavePath
	/*맵 위에 배치한 오브젝트들은 맵의 로컬 좌표계로 변환한 이후에 저장*/
	)&
{
	const Matrix ToMapLocal = FMath::Inverse(FMath::WorldMatrix(Scale, Rotation, Location));

	using namespace rapidjson;
	StringBuffer StrBuf;
	PrettyWriter<StringBuffer> Writer(StrBuf);
	// Cell Information Write...
	Writer.StartObject();

	Writer.Key("Decorators");
	Writer.StartArray();
	{
		for (auto& [DecoKey, CurDeco] : DecoratorContainer)
		{
			Writer.StartObject();
			{
				Writer.Key("FileName");
				Writer.String(ToA(DecoKey).c_str());

				Writer.Key("InstanceList");
				Writer.StartArray();
				for (auto& CurDecoInstance : CurDeco.Instances)
				{
					Writer.StartObject();

					Writer.Key("Scale");
					Writer.StartArray();

					Vector3 SaveScale = CurDecoInstance->Scale;
					SaveScale.x /= Scale.x;
					SaveScale.y /= Scale.y;
					SaveScale.z /= Scale.z;

					Writer.Double(SaveScale.x);
					Writer.Double(SaveScale.y);
					Writer.Double(SaveScale.z);
					Writer.EndArray();

					Writer.Key("Rotation");
					Writer.StartArray();
					const Vector3 SaveRotation = CurDecoInstance->Rotation;
					Writer.Double(SaveRotation.x);
					Writer.Double(SaveRotation.y);
					Writer.Double(SaveRotation.z);
					Writer.EndArray();

					Writer.Key("Location");
					Writer.StartArray();
					const Vector3 SaveLocation = FMath::Mul( CurDecoInstance->Location  , ToMapLocal );
					Writer.Double(SaveLocation.x);
					Writer.Double(SaveLocation.y);
					Writer.Double(SaveLocation.z);
					Writer.EndArray();

					Writer.Key("bLandscapeInclude");
					Writer.Bool(CurDecoInstance->bLandscapeInclude);

					Writer.EndObject();
				}
				Writer.EndArray();
			}
			Writer.EndObject();
		}
	}
	Writer.EndArray();
	

	Writer.EndObject();

	std::ofstream Of{ SavePath };
	DecoratorSaveInfo = StrBuf.GetString();
	Of << DecoratorSaveInfo;
}

void Engine::Landscape::Load(const std::filesystem::path& LoadPath)&
{
	std::ifstream Is{ LoadPath };

	using namespace rapidjson;

	if (!Is.is_open()) return;

	IStreamWrapper Isw(Is);
	Document _Document;
	_Document.ParseStream(Isw);

	if (_Document.HasParseError())
	{
		MessageBox(Engine::Global::Hwnd, L"Json Parse Error", L"Json Parse Error", MB_OK);
		return;
	}

	const Matrix MapWorld = FMath::WorldMatrix(Scale, Rotation, Location);
	const Value& DecoratorValue  = _Document["Decorators"];
	const auto& DecoArray  =  DecoratorValue.GetArray();

	for (auto DecoIterator = DecoArray.begin();
		DecoIterator != DecoArray.end(); ++DecoIterator)
	{
		const std::string FileName = DecoIterator->FindMember("FileName")->value.GetString();
		const auto& InstanceList = DecoIterator->FindMember("InstanceList")->value.GetArray();

		for (auto InstanceIter = InstanceList.begin();
			InstanceIter != InstanceList.end(); ++InstanceIter)
		{
			const auto& ScaleArr = InstanceIter->FindMember("Scale")->value.GetArray();
			const auto& RotationArr = InstanceIter->FindMember("Rotation")->value.GetArray();
			const auto& LocationArr = InstanceIter->FindMember("Location")->value.GetArray();
			const bool bLoadLandscapeInclude = InstanceIter->FindMember("bLandscapeInclude")->value.GetBool();

			Vector3 LoadScale { ScaleArr[0].GetFloat(), 
									  ScaleArr[1].GetFloat(),
									  ScaleArr[2].GetFloat() };

			const Vector3 LoadRotation{  RotationArr[0].GetFloat(),
									  RotationArr[1].GetFloat(),
									  RotationArr[2].GetFloat() };

			Vector3 LoadLocation{  LocationArr[0].GetFloat(),
									  LocationArr[1].GetFloat(),
									  LocationArr[2].GetFloat() };

			LoadScale.x *= Scale.x;
			LoadScale.y *= Scale.y;
			LoadScale.z *= Scale.z;

			LoadLocation = FMath::Mul(LoadLocation, MapWorld); 

			PushDecorator(ToW(FileName), LoadScale, LoadRotation, LoadLocation, bLoadLandscapeInclude);
		}
	};

}


// 랜드스케이프 인클루드 해결 해야함.