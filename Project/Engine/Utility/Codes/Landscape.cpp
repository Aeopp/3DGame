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
#include "Timer.h"


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



static bool IsFloatingDecorator(const std::wstring& DecoratorKey)
{
	static std::array<std::wstring, 3u> FloatingDecoratorNames
	{
		 L"Floating",
		L"Rock",
		L"BGIslandSet01"
	};
	for (const auto& FloatingDecoName : FloatingDecoratorNames)
	{
		if (DecoratorKey.find(FloatingDecoName) != std::string::npos)
		{
			return true;
		}
	}

	return false;
}

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
	if (IsFloatingDecorator(LoadFileName))
	{
		LoadDecorator._Option = Engine::Landscape::Decorator::Option::Floating;
	}


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

		const std::wstring
			MatName = ToW(AiScene->mMaterials[AiMesh->mMaterialIndex]->GetName().C_Str());

		LoadDecorator.Meshes[MeshIdx].MaterialInfo.Load(Device,
			LoadPath / L"Material",MatName + L".mat", L".tga");
	};

	DecoratorContainer.insert({ LoadFileName,LoadDecorator } );
}



std::pair< std::weak_ptr<typename Engine::Landscape::DecoInformation>  ,std::wstring >
Engine::Landscape::PushDecorator(
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
		DecoInfoInstance.bLandscapeInclude = bLandscapePolygonInclude;
		
		if (Deco._Option == Engine::Landscape::Decorator::Option::Floating)
		{
			Engine::Landscape::FloatingInformation  _FloatValue{};
			_FloatValue.Initialize();
			DecoInfoInstance.OptionValue = _FloatValue;
		}

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

		auto SharedDecoInfo=std::make_shared<Engine::Landscape::DecoInformation>(DecoInfoInstance);
		Deco.Instances.push_back(SharedDecoInfo);

		return { SharedDecoInfo ,Key };
	}

	return {};
}

std::pair< std::weak_ptr<typename Engine::Landscape::DecoInformation>, std::wstring >
Engine::Landscape::PushDecorator(const std::wstring DecoratorKey, const Vector3& Scale, const Vector3& Rotation, const Vector3& Location, const bool bLandscapePolygonInclude, const Ray WorldRay)&
{
	std::map<float, Vector3> IntersectResults{};
	for (const auto& CurWorldPlane : WorldPlanes)
	{
		float t;  Vector3 IntersectPt{}; 
		if (FMath::IsTriangleToRay(CurWorldPlane, WorldRay, t, IntersectPt))
		{
			IntersectResults[t] = IntersectPt;
		}
	}
	if (false == IntersectResults.empty())
	{
		const Vector3 TheMostNearIntersectPoint = IntersectResults.begin()->second;
		return PushDecorator(DecoratorKey, Scale, Rotation, TheMostNearIntersectPoint + Location, bLandscapePolygonInclude);
	}
	
	return  {};
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

std::pair<std::weak_ptr<typename Engine::Landscape::DecoInformation>,std::wstring>
	Engine::Landscape::PickDecoInstance(const Ray WorldRay)&
{
	std::map<float, std::pair <std::weak_ptr<Engine::Landscape::DecoInformation>, std::wstring>>
		PickResults{};
	for (auto& [Key , CurDeco ] : DecoratorContainer)
	{
		for (auto& CurDecoMesh : CurDeco.Meshes)
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
							PickResults[t] = { CurInstance,Key };
						}
					}
				}
			}
		}
	}

	if (!PickResults.empty())
	{
		auto TheMostNearDecoInstanceIter= PickResults.begin();
		PickDecoInstancePtr = TheMostNearDecoInstanceIter->second.first.lock().get();
		return TheMostNearDecoInstanceIter->second;
	}
	
	return {};
}

std::optional<Vector3 > Engine::Landscape::RayIntersectPoint(const Ray WorldRay) const&
{
	std::map<float, Vector3> IntersectResults{};
	for (const auto& CurWorldPlane : WorldPlanes)
	{
		float t;  Vector3 IntersectPt{};
		if (FMath::IsTriangleToRay(CurWorldPlane, WorldRay, t, IntersectPt))
		{
			IntersectResults[t] = IntersectPt;
			
		}
	}

	if (IntersectResults.empty()==false)
	{
		return { IntersectResults.begin()->second };
	}
	
	return {};
}




// TODO :: 노말매핑 사용시 버텍스 타입 변경해야 하며. FVF 변경도 고려 해야함 . 
// 또한 템플릿 코드로 변경시 리소스 시스템 접근 방식 변경 해야할 수도 있음 . 
void Engine::Landscape::Initialize(
	IDirect3DDevice9* const Device,
	const Vector3 Scale,
	const Vector3 Rotation,
	const Vector3 Location)&
{
	this->Device = Device;
	this->Scale = Scale;
	this->Rotation = Rotation;
	this->Location = Location;

	const Matrix MapWorld = FMath::WorldMatrix(this->Scale,Rotation, Location);
	auto& ResourceSys = ResourceSystem::Instance;
	using VertexType = Vertex::LocationTangentUV2D;
	std::string  VtxTypeName = typeid(VertexType).name();
	std::wstring VtxTypeWName;
	VtxTypeWName.assign(std::begin(VtxTypeName), std::end(VtxTypeName));
	VtxDecl  = ResourceSys->Get<IDirect3DVertexDeclaration9>(VtxTypeWName);

	if (!VtxDecl)
	{
		VtxDecl = VertexType::GetVertexDecl(Device);
		ResourceSys->Insert<IDirect3DVertexDeclaration9>(VtxTypeWName , VtxDecl);
	}


	ForwardShaderFx.Initialize(L"LandscapeFx");
	DeferredAlbedoNormalWorldPosDepthSpecular.Initialize(L"DeferredAlbedoNormalWorldPosDepthSpecularFx");
	DeferredRimFx.Initialize(L"DeferredRimFx");
	ShadowDepthFx.Initialize(L"ShadowDepthFx");
}

void Engine::Landscape::Tick(const float Tick)&
{
	if (Engine::Global::bDebugMode)
	{
		for (auto& [DecoKey, CurDeco] : DecoratorContainer)
		{
			CurDeco.Instances.erase(std::remove_if(std::begin(CurDeco.Instances), std::end(CurDeco.Instances),
				[](auto& TargetInstance) {
					return TargetInstance->bPendingKill;
				}), std::end(CurDeco.Instances));
		}
	}

	for ( auto& [DecoKey, CurDeco] : DecoratorContainer)
	{
		const bool IsFloatingDeco = CurDeco._Option == Engine::Landscape::Decorator::Option::Floating;

		for ( auto& CurDecoInstance : CurDeco.Instances)
		{
			if (IsFloatingDeco)
			{
				CurDecoInstance->Location = 
					std::any_cast<Engine::Landscape::FloatingInformation&>
					(CurDecoInstance->OptionValue).Floating(Tick, CurDecoInstance->Location);
			}
		};
	};
}


void Engine::Landscape::Render(Engine::Frustum& RefFrustum,
	const Matrix& View, const Matrix& Projection,
	const Vector4& CameraLocation4D ,
	IDirect3DTexture9* const ShadowDepthMap  ,
	const Matrix& LightViewProjection,
	const float ShadowDepthMapSize,
	const float ShadowDepthBias ,
	const Vector3& FogColor ,
	const float FogDistance )&
{
	if (Engine::Global::bDebugMode)
	{
		ImGui::Begin("Floating");
		if (ImGui::Button("FloatingDecoInstancesReInit"))
		{
			FloatingDecoInstancesReInit();
		}
		Engine::Landscape::FloatingInformation::RangeEdit();
		ImGui::End();
	};

	if (nullptr == Device)
		return;

	auto& Renderer = *Engine::Renderer::Instance;
	const Matrix MapWorld = FMath::WorldMatrix(Scale, Rotation, Location);

	auto Fx = ForwardShaderFx.GetHandle();
	Fx->SetFloat("FogDistance", FogDistance);
	Fx->SetFloatArray("FogColor", FogColor, 3u);
	Fx->SetMatrix("View", &View);
	Fx->SetMatrix("Projection", &Projection);
	Fx->SetVector("LightDirection", &Renderer._DirectionalLight._LightInfo.Direction);
	Fx->SetVector("LightColor", &Renderer._DirectionalLight._LightInfo.LightColor);
	Fx->SetVector("CameraLocation", &CameraLocation4D);
	Fx->SetTexture("ShadowDepthMap", ShadowDepthMap);
	Fx->SetMatrix("LightViewProjection", &LightViewProjection);
	Fx->SetFloat("ShadowDepthMapSize", ShadowDepthMapSize);
	Fx->SetFloat("ShadowDepthBias", ShadowDepthBias);


	Device->SetVertexDeclaration(VtxDecl);

	for (const auto& [DecoKey, CurDeco] : DecoratorContainer)
	{
		const bool IsFloatingDeco = CurDeco._Option == Engine::Landscape::Decorator::Option::Floating;

		for (const auto& CurDecoInstance : CurDeco.Instances)
		{
			const Vector3 DecoTfmScale = CurDecoInstance->Scale;
			const Vector3 DecoTfmLocation = CurDecoInstance->Location;
			const Vector3 DecoTfmRotation = CurDecoInstance->Rotation;

			const Matrix DecoWorld = 
				FMath::WorldMatrix(
					DecoTfmScale, 
					DecoTfmRotation, DecoTfmLocation);

			uint32 PassNum = 0u;
			Fx->Begin(&PassNum, 0);
		
			for (auto& CurMesh : CurDeco.Meshes)
			{
				if (false == CurMesh.MaterialInfo.bForwardRender)continue;

				const bool bRender = CurDecoInstance->CurRenderIDSet.contains(CurMesh.ID);

				if (bRender )
				{
					Device->SetStreamSource(0, CurMesh.VtxBuf, 0, CurMesh.Stride);
					Device->SetIndices(CurMesh.IdxBuf);
					Fx->SetMatrix("World", &DecoWorld);
					Fx->SetVector("RimAmtColor", &CurMesh.MaterialInfo.RimAmtColor);
					Fx->SetFloat("RimOuterWidth", CurMesh.MaterialInfo.RimOuterWidth);
					Fx->SetFloat("RimInnerWidth", CurMesh.MaterialInfo.RimInnerWidth);
					Fx->SetFloat("Power", CurMesh.MaterialInfo.Power);
					Fx->SetFloat("SpecularIntencity", CurMesh.MaterialInfo.SpecularIntencity);
					Fx->SetFloat("Contract", CurMesh.MaterialInfo.Contract);
					Fx->SetFloat("DetailScale", CurMesh.MaterialInfo.DetailScale);
					Fx->SetFloat("DetailDiffuseIntensity", CurMesh.MaterialInfo.DetailDiffuseIntensity);
					Fx->SetFloat("DetailNormalIntensity", CurMesh.MaterialInfo.DetailNormalIntensity);
					Fx->SetFloat("CavityCoefficient", CurMesh.MaterialInfo.CavityCoefficient);
					Fx->SetFloat("AlphaAddtive", CurMesh.MaterialInfo.AlphaAddtive);

					if (Engine::Global::bDebugMode
						&& (PickDecoInstancePtr == CurDecoInstance.get()))
					{
						const Vector4 PickAccentAmbient{ 25.f,0.f,255.f,1.f };
						Fx->SetVector("AmbientColor", &PickAccentAmbient);
					}
					else
					{
						Fx->SetVector("AmbientColor", &CurMesh.MaterialInfo.AmbientColor);
					}
					
					 CurMesh.MaterialInfo.BindingTexture(Fx);
					 Fx->CommitChanges();

					for (uint32 i = 0; i < PassNum; ++i)
					{
						Fx->BeginPass(i);
						Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, CurMesh.VtxCount,
							0u, CurMesh.PrimitiveCount);
						Fx->EndPass();
					}
				}
			}

			Fx->End();
		}
	}
	
	if (Engine::Global::bDebugMode && bDecoratorSphereMeshRender)
	{
		auto& _ResourceSys = *ResourceSystem::Instance;
		ID3DXMesh* DebugSphere = _ResourceSys.Get<ID3DXMesh>(L"SphereMesh");
		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

		for (const auto& [DecoKey, CurDeco] : DecoratorContainer)
		{
			for (const auto& CurDecoInstance : CurDeco.Instances)
			{
				const Vector3 DecoTfmScale    = CurDecoInstance->Scale;
				const Vector3 DecoTfmLocation = CurDecoInstance->Location;
				const Vector3  DecoTfmRotation = CurDecoInstance->Rotation;

				for (const auto& _Mesh :CurDeco.Meshes)
				{
					const bool bRender = CurDecoInstance->CurRenderIDSet.contains(_Mesh.ID); 
					if (bRender)
					{
						const Matrix SphereLocalMatrix = FMath::WorldMatrix(
							_Mesh.BoundingSphere.Radius
							, { 0,0,0 },
							_Mesh.BoundingSphere.Center);

						const Matrix DecoWorld = SphereLocalMatrix * FMath::WorldMatrix(
							FMath::MaxScala(DecoTfmScale),
							DecoTfmRotation, DecoTfmLocation);

						Device->SetTransform(D3DTS_WORLD, &DecoWorld);
						DebugSphere->DrawSubset(0u);
					}
				}
			}
		}

		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}
}

void Engine::Landscape::FrustumCullingCheck(Engine::Frustum& RefFrustum)&
{
	if (Engine::Global::bDebugMode)
		ImGui::Begin("Frustum Culling Log");

	for ( auto& [DecoKey, CurDeco] : DecoratorContainer)
	{
		for (auto& CurDecoInstance : CurDeco.Instances)
		{
			CurDecoInstance->CurRenderIDSet.clear();

			const Vector3 DecoTfmScale = CurDecoInstance->Scale;
			const Vector3 DecoTfmLocation = CurDecoInstance->Location;
			const Vector3 DecoTfmRotation = CurDecoInstance->Rotation;

			const Matrix DecoWorld =
				FMath::WorldMatrix(
					DecoTfmScale,
					DecoTfmRotation, DecoTfmLocation);

			for (auto& CurMesh : CurDeco.Meshes)
			{
				Sphere CurDecoMeshBoundingSphere;
				CurDecoMeshBoundingSphere.Center = FMath::Mul(CurMesh.BoundingSphere.Center, DecoWorld);
				CurDecoMeshBoundingSphere.Radius = (CurMesh.BoundingSphere.Radius * FMath::MaxScala(DecoTfmScale));

				const bool bRender = RefFrustum.IsIn(CurDecoMeshBoundingSphere); 
				if (bRender)
				{
					CurDecoInstance->CurRenderIDSet.insert(CurMesh.ID);
				}

				if (Engine::Global::bDebugMode)
				{
					if (bRender)
					{
						ImGui::TextColored(ImVec4{ 1.f,107.f / 255.f,181.f / 255.f,0.5f }, "Draw : %s", CurMesh.Name.c_str());
					}
					else
					{
						ImGui::TextColored(ImVec4{ 158.f / 255.f,158.f / 255.f,255.f,0.5f }, "Culling : %s", CurMesh.Name.c_str());
					}
				}
			}
		}
	}

	if (Engine::Global::bDebugMode)
		ImGui::End();

}

void Engine::Landscape::RenderDeferredAlbedoNormalWorldPosDepthSpecular(Engine::Frustum& RefFrustum, const Matrix& View, const Matrix& Projection, const Vector4& CameraLocation)&
{
	if (nullptr == Device)
		return;

	auto& Renderer = *Engine::Renderer::Instance;
	const Matrix MapWorld = FMath::WorldMatrix(Scale, Rotation, Location);

	Device->SetVertexDeclaration(VtxDecl);
	

	auto Fx = DeferredAlbedoNormalWorldPosDepthSpecular.GetHandle();
	Fx->SetMatrix("View", &View);
	Fx->SetMatrix("Projection", &Projection);

	for (const auto& [DecoKey, CurDeco] : DecoratorContainer)
	{
		for (const auto& CurDecoInstance : CurDeco.Instances)
		{
			const Vector3 DecoTfmScale = CurDecoInstance->Scale;
			const Vector3 DecoTfmLocation = CurDecoInstance->Location;

			const Vector3 DecoTfmRotation = CurDecoInstance->Rotation;

			const Matrix DecoWorld =
				FMath::WorldMatrix(
					DecoTfmScale,
					DecoTfmRotation, DecoTfmLocation);

			uint32 PassNum = 0u;
			Fx->Begin(&PassNum, 0);

			for (auto& CurMesh : CurDeco.Meshes)
			{
				if (true == CurMesh.MaterialInfo.bForwardRender)continue;

				const bool bRender = CurDecoInstance->CurRenderIDSet.contains(CurMesh.ID);

				if (bRender)
				{

					Device->SetStreamSource(0, CurMesh.VtxBuf, 0, CurMesh.Stride);
					Device->SetIndices(CurMesh.IdxBuf);
					Fx->SetFloat("DetailScale", CurMesh.MaterialInfo.DetailScale);
					Fx->SetFloat("Contract", CurMesh.MaterialInfo.Contract);
					Fx->SetMatrix("World", &DecoWorld);
					Fx->SetFloat("Power", CurMesh.MaterialInfo.Power);
					Fx->SetFloat("SpecularIntencity", CurMesh.MaterialInfo.SpecularIntencity);
					Fx->SetFloat("DetailDiffuseIntensity", CurMesh.MaterialInfo.DetailDiffuseIntensity);
					Fx->SetFloat("DetailNormalIntensity", CurMesh.MaterialInfo.DetailNormalIntensity);
					Fx->SetFloat("CavityCoefficient", CurMesh.MaterialInfo.CavityCoefficient);

					CurMesh.MaterialInfo.BindingTexture(Fx);
					Fx->CommitChanges();

					for (uint32 i = 0; i < PassNum; ++i)
					{
						Fx->BeginPass(i);
						Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, CurMesh.VtxCount,
							0u, CurMesh.PrimitiveCount);
						Fx->EndPass();
					}
				}
			}

			Fx->End();
		}
	}
}

void Engine::Landscape::RenderShadowDepth(
	const Matrix& LightViewProjection)&
{
	if (nullptr == Device)
		return;

	auto& Renderer = *Engine::Renderer::Instance;
	Device->SetVertexDeclaration(VtxDecl);

	auto Fx = ShadowDepthFx.GetHandle();
	Fx->SetMatrix("LightViewProjection", &LightViewProjection);

	for (const auto& [DecoKey, CurDeco] : DecoratorContainer)
	{
		for (const auto& CurDecoInstance : CurDeco.Instances)
		{
			const Vector3 DecoTfmScale = CurDecoInstance->Scale;
			const Vector3 DecoTfmLocation = CurDecoInstance->Location;
			const Vector3 DecoTfmRotation = CurDecoInstance->Rotation;

			const Matrix DecoWorld =
				FMath::WorldMatrix(
					DecoTfmScale,
					DecoTfmRotation, DecoTfmLocation);

			uint32 PassNum = 0u;
			Fx->Begin(&PassNum, 0);

			for (auto& CurMesh : CurDeco.Meshes)
			{
				Device->SetStreamSource(0, CurMesh.VtxBuf, 0, CurMesh.Stride);
				Device->SetIndices(CurMesh.IdxBuf);
				Fx->SetMatrix("World", &DecoWorld);

				Fx->CommitChanges();

				for (uint32 i = 0; i < PassNum; ++i)
				{
					Fx->BeginPass(i);
					Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, CurMesh.VtxCount,
						0u, CurMesh.PrimitiveCount);
					Fx->EndPass();
				}
			}

			Fx->End();
		}
	}
}

void Engine::Landscape::RenderDeferredRim(Engine::Frustum& RefFrustum,
	const Matrix& View, const Matrix& Projection, const Vector4& CameraLocation)&
{
	if (nullptr == Device)
		return;

	auto& Renderer = *Engine::Renderer::Instance;
	const Matrix MapWorld = FMath::WorldMatrix(Scale, Rotation, Location);

	Device->SetVertexDeclaration(VtxDecl);

	auto Fx = DeferredRimFx.GetHandle();
	Fx->SetMatrix("View", &View);
	Fx->SetMatrix("Projection", &Projection);

	for (const auto& [DecoKey, CurDeco] : DecoratorContainer)
	{
		for (const auto& CurDecoInstance : CurDeco.Instances)
		{
			const Vector3 DecoTfmScale = CurDecoInstance->Scale;
			const Vector3 DecoTfmLocation = CurDecoInstance->Location;

			const Vector3 DecoTfmRotation = CurDecoInstance->Rotation;

			const Matrix DecoWorld =
				FMath::WorldMatrix(
					DecoTfmScale,
					DecoTfmRotation, DecoTfmLocation);

			uint32 PassNum = 0u;
			Fx->Begin(&PassNum, 0);

			for (auto& CurMesh : CurDeco.Meshes)
			{
				const bool bRender = CurDecoInstance->CurRenderIDSet.contains(CurMesh.ID);
				if (bRender)
				{
					Device->SetStreamSource(0, CurMesh.VtxBuf, 0, CurMesh.Stride);
					Device->SetIndices(CurMesh.IdxBuf);
					Fx->SetMatrix("World", &DecoWorld);
					Fx->SetFloat("RimOuterWidth", CurMesh.MaterialInfo.RimOuterWidth);
					Fx->SetFloat("RimInnerWidth", CurMesh.MaterialInfo.RimInnerWidth);
					Fx->SetVector("RimAmtColor",&CurMesh.MaterialInfo.RimAmtColor);

					Fx->CommitChanges();

					for (uint32 i = 0; i < PassNum; ++i)
					{
						Fx->BeginPass(i);
						Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, CurMesh.VtxCount,
							0u, CurMesh.PrimitiveCount);
						Fx->EndPass();
					}
				}
			}

			Fx->End();
		}
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

void Engine::Landscape::Clear()&
{
	std::for_each(std::begin(DecoratorContainer), std::end(DecoratorContainer),
		[](decltype(DecoratorContainer)::value_type& DecoKey_Decorator)
		{
			DecoKey_Decorator.second.Instances.clear();
		});
};

void Engine::Landscape::ReInitWorldPlanes()&
{
	WorldPlanes.clear();

	for (const auto& [DecoKey, _Deco] : DecoratorContainer)
	{
		std::vector<Matrix> CurDecoInstancesWorld{};

		for (const auto& _CurDecoInstance : _Deco.Instances)
		{
			if (_CurDecoInstance->bLandscapeInclude)
			{
				CurDecoInstancesWorld.push_back(
					FMath::WorldMatrix(_CurDecoInstance->Scale,
						_CurDecoInstance->Rotation,
						_CurDecoInstance->Location));
			}
		}

		for (const auto& CurDecoMesh : _Deco.Meshes)
		{
			byte* VtxBufPtr{ nullptr };
			CurDecoMesh.VtxBuf->Lock(0u, 0u, reinterpret_cast<void**>(&VtxBufPtr), NULL);
			for (const Matrix& InstanceWorld : CurDecoInstancesWorld)
			{
				for (uint32 i = 0; i < CurDecoMesh.VtxCount; i += 3)
				{
					std::array<Vector3, 3u> WorldPoints{};

					for (uint32 j = 0; j < WorldPoints.size(); ++j)
					{
						WorldPoints[j] =
							*reinterpret_cast<Vector3*>(VtxBufPtr + ((i + j) * CurDecoMesh.Stride));
						WorldPoints[j] = FMath::Mul(WorldPoints[j], InstanceWorld);
					};

					WorldPlanes.push_back(PlaneInfo::Make(WorldPoints));
				}
			}
			CurDecoMesh.VtxBuf->Unlock();
		}
	}
}
void Engine::Landscape::FloatingDecoInstancesReInit()&
{
	for (auto& [Key, Deco] : DecoratorContainer)
	{
		if (Deco._Option == Engine::Landscape::Decorator::Option::Floating)
		{
			for (auto& CurDecoInstance : Deco.Instances)
			{
				std::any_cast<Engine::Landscape::FloatingInformation&>(CurDecoInstance->OptionValue).Initialize();
			}
		}
	}
};

void Engine::Landscape::FloatingInformation::RangeEdit()
{
	ImGui::SliderFloat2("VibrationWidthRange", (float*)&VibrationWidthRange, 1.f, 100.f);
	ImGui::SliderFloat2("RotationAccRange", (float*)&RotationAccRange, 0.001f, 1.f);
	ImGui::SliderFloat2("VibrationAccRange", (float*)&VibrationAccRange, 0.001f, 5.f);
}

Engine::Landscape::Mesh::Mesh() 
{
	static uint32 CurrentID = 0u;
	ID = CurrentID++;
}
