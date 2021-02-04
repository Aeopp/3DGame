#include "SkeletonMesh.h"
#include "UtilityGlobal.h"
#include "FMath.hpp"
#include "ResourceSystem.h"
#include "ExportUtility.hpp"
#include "imgui.h"

void SkeletonMesh::Load(IDirect3DDevice9* const Device)&
{
	/// <summary>
	using Type = SkeletonVertex;
	const std::wstring ResourceName = L"InputNameHere";
	const std::filesystem::path FilePath = "..\\..\\..\\Resource\\Mesh\\DynamicMesh\\PlayerXfile\\";
	const std::filesystem::path FileName = "Player.x";
	/// </summary>
	this->Device = Device;
	// 모델 생성 플래그 , 같은 플래그를 두번, 혹은 호환이 안되는
	// 플래그가 겹칠 경우 런타임 에러이며 에러 핸들링이
	// 어려우므로 매우 유의 할 것.
	AiScene = Engine::Global::AssimpImporter.ReadFile(
		(FilePath/FileName).string(),
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
		aiProcess_LimitBoneWeights |
		aiProcess_GenSmoothNormals |
		aiProcess_SortByPType |
		aiProcess_OptimizeMeshes |
		aiProcess_OptimizeGraph
	);

	static uint32 SkeletonResourceID = 0u;
	auto& ResourceSys = RefResourceSys();

	// Bone Info 
	Bone Root;
	Root.Name = AiScene->mRootNode->mName.C_Str();
	Root.OriginTransform=Root.Transform = FromAssimp(AiScene->mRootNode->mTransformation);
	Root.Parent = nullptr;
	Root.ToRoot = FMath::Identity();
	std::shared_ptr<Bone> _BoneShared = std::make_shared<Bone>(Root);
	BoneTable.insert({Root.Name,_BoneShared});
	for (uint32 i = 0; i < AiScene->mRootNode->mNumChildren; ++i)
	{
		_BoneShared->Childrens.push_back(MakeHierarchy(_BoneShared.get(), AiScene->mRootNode->mChildren[i]));
	}

	RootBone = _BoneShared.get();

	for (uint32 MeshIdx = 0u; MeshIdx < AiScene->mNumMeshes; ++MeshIdx)
	{
		const std::wstring CurrentResourceName =
			ResourceName + L"_" + std::to_wstring(SkeletonResourceID++);

		Mesh CreateMesh{};
		aiMesh* _AiMesh = AiScene->mMeshes[MeshIdx];
		// 버텍스 버퍼.
		std::vector<Type> Verticies{};
		for (uint32 VerticesIdx = 0u; VerticesIdx < _AiMesh->mNumVertices; ++VerticesIdx)
		{
			Verticies.push_back(Type::MakeFromAssimpMesh(_AiMesh, VerticesIdx));
		}
		
		const std::wstring MeshVtxBufResourceName =
			L"SkeletonMesh_VertexBuffer_" + CurrentResourceName;
		IDirect3DVertexBuffer9* _VertexBuffer{ nullptr };
		const uint32 VtxBufSize = sizeof(Type) * Verticies.size();
		Device->CreateVertexBuffer(VtxBufSize,D3DUSAGE_DYNAMIC, Type::FVF,
			D3DPOOL_DEFAULT, &_VertexBuffer, nullptr);
		CreateMesh.VertexBuffer =  ResourceSys.Insert<IDirect3DVertexBuffer9>(MeshVtxBufResourceName, _VertexBuffer);
		CreateMesh.PrimitiveCount=CreateMesh.FaceCount = _AiMesh->mNumFaces;
		Type* VertexBufferPtr{ nullptr };
		CreateMesh.VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBufferPtr), NULL);
		std::memcpy(VertexBufferPtr, Verticies.data(), VtxBufSize);
		CreateMesh.VertexBuffer->Unlock();
		CreateMesh.VtxCount = Verticies.size();

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
			L"SkeletonMesh_IndexBuffer_" + CurrentResourceName;
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
		 CreateMesh.Verticies = Verticies;
		// 머테리얼.
		aiMaterial* AiMaterial = AiScene->mMaterials[_AiMesh->mMaterialIndex];
		if (AiMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType_DIFFUSE, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == AI_SUCCESS)
			{
				const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();
				const std::wstring TexResourceName = L"SkeletonMesh_DiffuseTexture_" + CurrentResourceName;

				RefResourceSys().Emplace<IDirect3DTexture9>
					(TexResourceName,
					D3DXCreateTextureFromFile, Device, TexFileFullPath.c_str(), &CreateMesh.DiffuseTexture);
			}
		}
		if (AiMaterial->GetTextureCount(aiTextureType::aiTextureType_NORMALS) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == AI_SUCCESS)
			{
				const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();
				const std::wstring TexResourceName = L"SkeletonMesh_NormalTexture_" + CurrentResourceName;

				RefResourceSys().Emplace<IDirect3DTexture9>
					(TexResourceName,
						D3DXCreateTextureFromFile, Device, TexFileFullPath.c_str(), &CreateMesh.NormalTexture);
			}
		}
		if (AiMaterial->GetTextureCount(aiTextureType::aiTextureType_SPECULAR) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType::aiTextureType_SPECULAR, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == AI_SUCCESS)
			{
				const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();
				const std::wstring TexResourceName = L"SkeletonMesh_SpecularTexture_" + CurrentResourceName;

				RefResourceSys().Emplace<IDirect3DTexture9>
					(TexResourceName,
						D3DXCreateTextureFromFile, Device, TexFileFullPath.c_str(), &CreateMesh.SpecularTexture);
			}
		}

		// Vtx Bone 정보,
		if (_AiMesh->HasBones())
		{
			CreateMesh.Offsets.resize(CreateMesh.VtxCount);
			CreateMesh.Weights.resize(CreateMesh.VtxCount);
			CreateMesh.Finals. resize(CreateMesh.VtxCount);
			for (uint32 BoneIdx = 0u; BoneIdx < _AiMesh->mNumBones; ++BoneIdx)
			{
				const aiBone* const CurVtxBone = _AiMesh->mBones[BoneIdx];
				auto iter = BoneTable.find(CurVtxBone->mName.C_Str());
				if (iter != std::end(BoneTable))
				{
					for (uint32 WeightIdx = 0u; WeightIdx < CurVtxBone->mNumWeights; ++WeightIdx)
					{
						const aiVertexWeight  _AiVtxWit = CurVtxBone->mWeights[WeightIdx];
						const uint32 VtxIdx = _AiVtxWit.mVertexId;
						const float _Wit =    _AiVtxWit.mWeight;
						const Matrix OffsetMatrix = FromAssimp(CurVtxBone->mOffsetMatrix);
						iter->second->Offset = OffsetMatrix;
						CreateMesh.Offsets[VtxIdx].push_back(OffsetMatrix);
						CreateMesh.Weights[VtxIdx].push_back(_Wit);
						CreateMesh.Finals [VtxIdx].push_back(&(iter->second->Final));
					}
				}
			}
		}
		MeshContainer.push_back(CreateMesh);
	}
}

void Bone::BoneMatrixUpdate(
	const uint32 TargetAnimIdx,
	const aiScene*const AiScene,
	Bone* Parent,
	const float T,
	const uint32 ScaleIdx,
	const uint32 RotationIdx,
	const uint32 PositionIdx)&
{
	if (!Parent)return;

	Matrix AnimationTransform = FMath::Identity();
	
	bool bBoneAnimation = false;

	if (AiScene->HasAnimations())
	{
		aiAnimation* _Animation = AiScene->mAnimations[TargetAnimIdx];
		if (_Animation)
		{
			ImGui::Text("TickPerSecond : %f",_Animation->mTicksPerSecond);
			ImGui::Text("Duration : %f",_Animation->mDuration);
			for (uint32 Channel = 0; Channel < _Animation->mNumChannels; ++Channel)
			{
				aiNodeAnim* _NodeAnim = _Animation->mChannels[Channel];
				
				if (std::string(_NodeAnim->mNodeName.C_Str())==this->Name)
				{
					bBoneAnimation = true;

					const aiVectorKey ScaleLhs = (_NodeAnim->mScalingKeys[ScaleIdx
					                                 %_NodeAnim->mNumScalingKeys]);
					const aiVectorKey ScaleRhs = (_NodeAnim->mScalingKeys[(ScaleIdx +1)
					                                 %_NodeAnim->mNumScalingKeys]);
					const aiQuatKey   QuatLhs =  (_NodeAnim->mRotationKeys[RotationIdx
						                             %_NodeAnim->mNumRotationKeys]);
					const aiQuatKey   QuatRhs =  (_NodeAnim->mRotationKeys[(RotationIdx +1)
												     %_NodeAnim->mNumRotationKeys]);
					const aiVectorKey PosLhs =   (_NodeAnim->mPositionKeys[PositionIdx 
						                            % _NodeAnim->mNumPositionKeys]);
					const aiVectorKey PosRhs =   (_NodeAnim->mPositionKeys[(PositionIdx +1) 
						                           % _NodeAnim->mNumPositionKeys]);

					AnimationTransform *= FMath::Scale(FMath::Lerp(FromAssimp(ScaleLhs.mValue), FromAssimp(ScaleRhs.mValue), 0.5f));
					AnimationTransform *= FMath::Rotation(FMath::Lerp(FromAssimp(QuatLhs.mValue), FromAssimp(QuatRhs.mValue), 0.5f));
					AnimationTransform *= FMath::Translation(FMath::Lerp(FromAssimp(PosLhs.mValue), FromAssimp(PosRhs.mValue), 0.5f));
				}
			}
		}
	}

	//ToRoot    = Parent->ToRoot * Parent->Transform;
	//Transform = OriginTransform * AnimationTransform;
	//Final     = Offset  * AnimationTransform * ToRoot;
	Transform = /*OriginTransform * */bBoneAnimation ? AnimationTransform : OriginTransform;
	ToRoot    = Transform * Parent->ToRoot;
	Final = Offset * ToRoot;

	for (auto& ChildrenTarget : Childrens)
	{
		ChildrenTarget->BoneMatrixUpdate(TargetAnimIdx,AiScene,this,
			T, ScaleIdx,RotationIdx, PositionIdx);
	}
}

void BoneNamePrint(Bone& Target)
{
	ImGui::Text("%s", Target.Name.c_str());
	for (auto& children : Target.Childrens)
	{
		BoneNamePrint(*children);
	}
};

void SkeletonMesh::Render()&
{
	static constexpr double Delta = 1.f / 50.f;
	T += Delta;
	ImGui::Begin("..");
	ImGui::Text("...");
	BoneNamePrint(*RootBone);
	ImGui::End();
	/// <summary>
	using Type = SkeletonVertex;
	/// </summary>
	Device->SetFVF(Type::FVF);
	Device->SetVertexShader(nullptr);
	Device->SetPixelShader(nullptr);
	for (auto& CurrentRenderMesh : MeshContainer)
	{
		Device->SetTexture(0, CurrentRenderMesh.DiffuseTexture);
		Device->SetStreamSource(0, CurrentRenderMesh.VertexBuffer, 0, sizeof(Type));
		Device->SetIndices(CurrentRenderMesh.IndexBuffer);
		Type* VertexBufferPtr{ nullptr };

		CurrentRenderMesh.VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBufferPtr), NULL);
		std::memcpy(VertexBufferPtr,CurrentRenderMesh.Verticies.data(),
			sizeof(Type)*CurrentRenderMesh.Verticies.size());
		for (uint32 i = 0; i < CurrentRenderMesh.Verticies.size(); ++i)
		{
			Vector3 AnimLocation          { 0,0,0 };
			const Vector3 OriginLocation = VertexBufferPtr[i].Location;
			for (uint32 j = 0; j < CurrentRenderMesh.Finals[i].size(); ++j)
			{
				Vector3 _Location{ 0,0,0 };
				D3DXVec3TransformCoord(&_Location,&OriginLocation,CurrentRenderMesh.Finals[i][j]);
				AnimLocation+=_Location *= CurrentRenderMesh.Weights[i][j];
			}
			VertexBufferPtr[i].Location = AnimLocation;
		}
		
		Device->DrawIndexedPrimitive(
			D3DPT_TRIANGLELIST, 0u, 0u, CurrentRenderMesh.VtxCount, 0u, CurrentRenderMesh.PrimitiveCount); 
		CurrentRenderMesh.VertexBuffer->Unlock();
	}
}

Bone* SkeletonMesh::MakeHierarchy(Bone* BoneParent,const aiNode* const AiNode)
{
	std::shared_ptr<Bone> _BoneShared = std::make_shared<Bone>(Bone{});
	_BoneShared->Name = AiNode->mName.C_Str(); 
	_BoneShared->OriginTransform=_BoneShared->Transform = FromAssimp(AiNode->mTransformation);
	_BoneShared->Parent = BoneParent;
	_BoneShared->ToRoot = _BoneShared->OriginTransform * BoneParent->ToRoot;
	BoneTable.insert({ _BoneShared->Name,_BoneShared });
	for (uint32 i = 0; i < AiNode->mNumChildren; ++i)
	{
		_BoneShared->Childrens.push_back(MakeHierarchy(_BoneShared.get(), AiNode->mChildren[i]));
	}
	return _BoneShared.get();
}

void SkeletonMesh::UpdateTrackIndex(const float DeltaTime)&
{
	///*const float FrameDuration      = AiScene->mAnimations[AnimIdx]->mDuration;
	//const float TickPerSecond = AiScene->mAnimations[AnimIdx]->mTicksPerSecond;
	//const float Duration;*/

	//ImGui::Text("Duration : %f ", FrameDuration);
	//ImGui::Text("TickPerSecond : %f ", TickPerSecond);
	//ImGui::Text("Duration : %f ", Duration);

	//if (T > Duration)
	//{
	//	T = 0.0f;
	//	++TrackIdx;
	//	// TrackIdx %= AiScene->mAnimations[TrackIdx];
	//}
}
