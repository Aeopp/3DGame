#include "Model.h"
#include "ResourceSystem.h"
#include <iostream>


// assimp Node 클래스의 정보를 참조해서 로딩이 완료되었다고
// 가정하는 본 정보 테이블에서 타겟 본 정보를 초기화해서 넘겨준다.
bool Engine::Model::ReadSkeleton(Bone& BoneOutput,aiNode* Node, 
	//본인포테이블은 메쉬마다 가지고 있음 .
	std::unordered_map<std::string,  /*어떠한 메쉬가 참조하는 assimp 본 이름*/
	std::pair<int32/*본 아이디*/, /*본 오프셋 매트릭스(??)*/Matrix>>& BoneInfoTable)
{
	// 본 테이블에서 정보를 찾는데 성공 !!
	if (BoneInfoTable.find(Node->mName.C_Str()) != BoneInfoTable.end())
	{
		// 본 정보를 전부 채워 넣는다. 
		BoneOutput.Name = Node->mName.C_Str();
		BoneOutput.ID = BoneInfoTable[BoneOutput.Name].first;
		BoneOutput.Offset = BoneInfoTable[BoneOutput.Name].second;

		for (int32 i = 0; i < Node->mNumChildren; ++i)
		{
			// Node 의 자식들까지 전부 재귀로 순회하며 자식 정보까지
			// 채워넣는다. assimp 가 읽어온 트리 구조를
			// 사용자 정의 구조체 Bone 에도 똑같이 매칭 시켜 주는 것이다 . 
			Bone Child;
			ReadSkeleton(Child, Node->mChildren[i], BoneInfoTable);
			BoneOutput.Children.push_back(Child);
		}
		return true;
	}
	else // 노드의 이름으로 찾지 못함 (본 테이블에 정보가 없는 상황.)
	{
		// 해당 노드의 자식들에게서 정보를 찾는다. (최악의 경우 Leaf 까지 내려간다.)
		for (int i = 0; i < Node->mNumChildren; ++i)
		{
			if (ReadSkeleton(BoneOutput, Node->mChildren[i], BoneInfoTable))
			{
				return true;
			}
		}
	}

	// Leaf 까지 내려갔으나 정보를 찾지 못하였다.
	return false;
}

void Engine::Model::LOAD_MODEL(const aiScene* Scene, aiMesh* Mesh, std::vector<Vertex::Animation>& VerticesOutput,
	std::vector<uint32>& IndicesOutput, 
	Bone& SkeletonOutput, uint32& BoneCount)
{
	VerticesOutput = {};
	IndicesOutput = {};
	// 메쉬의 데이터 로딩
	for (uint32 i = 0; i < Mesh->mNumVertices; ++i)
	{
		Vertex::Animation _Vertex;
		Vector3 _Vector3;
		_Vector3.x = Mesh->mVertices[i].x;
		_Vector3.y = Mesh->mVertices[i].y;
		_Vector3.z = Mesh->mVertices[i].z;
		_Vertex.Location = _Vector3;

		_Vector3.x = Mesh->mNormals[i].x;
		_Vector3.y = Mesh->mNormals[i].y;
		_Vector3.z = Mesh->mNormals[i].z;
		_Vertex.Normal = _Vector3;

		Vector2 _Vector2;
		// 텍스쳐 좌표를 여러개 쓴다면 인덱스 수정.
		_Vector2.x = Mesh->mTextureCoords[0][i].x;
		_Vector2.y = Mesh->mTextureCoords[0][i].y;
		_Vertex.UV = _Vector2;

		_Vertex.BoneIds = Vector4{0,0,0,0};
		_Vertex.BoneWeights = Vector4{0,0,0,0};

		VerticesOutput.push_back(_Vertex);
	}

	// 본 데이터 로딩
	//               어떠한 메쉬가 참조하는 본 (assimp) 이름          본 아이디   
	//본인포테이블은 메쉬마다 가지고 있음 . 
	std::unordered_map<std::string, std::pair<int32,Matrix>> BoneInfo{};
	// 배열의 인덱스는 버텍스의 아이디 원소는 참조하는 본의(최대4개) 개수
	std::vector<uint32>BoneCounts{};
	BoneCounts.resize(VerticesOutput.size(), 0u);
	BoneCount = Mesh->mNumBones;
	
	// 본을 전부 순회.
	for (uint32 i = 0u; i < BoneCount; ++i)
	{
		// 오프셋 매트릭스를 저장. 
		aiBone* bone = Mesh->mBones[i];
		Matrix _Matrix;
		                    
		std::memcpy(&_Matrix, &bone->mOffsetMatrix
			/*바인드포즈의 메시공간에서 골격 공간으로 변환*/
			, sizeof(Matrix));
		BoneInfo[bone->mName.C_Str()] = { i, _Matrix};

		// 해당 본의 가중치 개수만큼 순회
		for (int32 j = 0; j < bone->mNumWeights; ++j)
		{
			// 본이 가지고 있는 버텍스 아이디와 매핑되어있는
			// 가중치 정보를 참조해서 
			// 버텍스들의 가중치와 본 참조 인덱스 정보를 채워 넣는다.  
			uint32 Id = bone->mWeights[j].mVertexId;
			float weight = bone->mWeights[j].mWeight;
			BoneCounts[Id]++;

			switch (BoneCounts[Id])
			{
			case 1:
				VerticesOutput[Id].BoneIds.x = i;
				VerticesOutput[Id].BoneWeights.x = weight;
				break;	
			case 2:
				VerticesOutput[Id].BoneIds.y = i;
				VerticesOutput[Id].BoneWeights.y = weight;
				break;
			case 3:
				VerticesOutput[Id].BoneIds.z = i;
				VerticesOutput[Id].BoneWeights.z = weight;
				break;
			case 4 :
				VerticesOutput[Id].BoneIds.w = i;
				VerticesOutput[Id].BoneWeights.w = weight;
				break;
			default :
				break;
			}
		}
	};

	// 가중치를 정규화 하며 가중치의 합이 반드시 1이 되도록 정보를 검증하고 갱신한다.
	for (int32 i = 0; i < VerticesOutput.size(); i++)
	{
		Vector4& BoneWeights = VerticesOutput[i].BoneWeights;
		float TotalWeight = 
			BoneWeights.x + BoneWeights.y + BoneWeights.z + BoneWeights.z;
		if (TotalWeight > 0.0f)
		{
			VerticesOutput[i].BoneWeights = Vector4
			{
				BoneWeights.x / TotalWeight , 
				BoneWeights.y / TotalWeight ,
				BoneWeights.z / TotalWeight ,
				BoneWeights.w / TotalWeight ,
			};
		}
	}

	// 왼손 좌표계 옵션으로 로딩 하였다면
	// 페이스가 참조하는 0,1,2 값이 삼각형의 그리는 순서와
	// 일치하므로 해당 정보로 인덱스 정보를 채워 넣는다.
	for (int32 i = 0; i < Mesh->mNumFaces; ++i)
	{
		aiFace& _Face = Mesh->mFaces[i];
		for (uint32 j = 0; j < _Face.mNumIndices; ++j)
		{
			IndicesOutput.push_back(_Face.mIndices[j]);
		}
	}
	
	// 본 테이블 정보가 채워진 이후에 스켈레톤 정보를 로딩한다. 씬의 루트노드로부터.
	ReadSkeleton(SkeletonOutput, Scene->mRootNode, BoneInfo);
}

void Engine::Model::LoadAnimation(const aiScene* Scene, Animation& _Animation)
{
	aiAnimation* _Anim = Scene->mAnimations[0];

	if (_Anim->mTicksPerSecond != 0.0f)
	{
		_Animation.TicksPerSecond = _Anim->mTicksPerSecond;
	}
	else
	{
		_Animation.TicksPerSecond = 1;
	}

	_Animation.Duration = _Anim->mDuration * _Anim->mTicksPerSecond;
	_Animation.BoneTransforms = {};
	// 각 뼈에 대한 위치 회전 스케일링 로딩
	// 채널 = 본
	for (int32 i = 0; i < _Anim->mNumChannels; ++i)
	{
		aiNodeAnim* Channel = _Anim->mChannels[i];
		BoneTransformTrack Track;
		for (int j = 0; j < Channel->mNumPositionKeys; ++j)
		{
			Track.PositionTimeStamps.push_back(Channel->mPositionKeys[j].mTime);
			Track.Positions.push_back(AssimpTo(Channel->mPositionKeys[j].mValue));
		}
		for (int j = 0; j < Channel->mNumRotationKeys; ++j)
		{
			Track.RotationTimeStamps.push_back(Channel->mRotationKeys[j].mTime);
			Track.Rotations.push_back(AssimpTo(Channel->mRotationKeys[j].mValue));
		}
		for (int j = 0; j < Channel->mNumScalingKeys; ++j)
		{
			Track.ScalTimeStamps.push_back(Channel->mScalingKeys[j].mTime);
			Track.Scales.push_back(AssimpTo(Channel->mScalingKeys[j].mValue));
		}
		_Animation.BoneTransforms[Channel->mNodeName.C_Str()] = Track;
	}
}

std::pair<uint32, float> Engine::Model::GetTimeFraction
          (std::vector<float>& Times, float& DeltaTime)
{
	uint32 Segment = 0u;
	while (DeltaTime > Times[Segment])
		++Segment; 

	float Start = Times[Segment - 1];
	float End = Times[Segment];
	float Frac = (DeltaTime - Start) / (End - Start);
	return { Segment,Frac };
}

void Engine::Model::GetPose(
	Animation& _Animation, 
	Bone& Skeleton, 
	float DeltaTime, 
	std::vector<Matrix>& Output, 
	Matrix& ParentTransform, 
	Matrix& GlobalInverseTransform)
{
	BoneTransformTrack& Btt = _Animation.BoneTransforms[Skeleton.Name];
	DeltaTime = std::fmod(DeltaTime,_Animation.Duration);
	std::pair<uint32, float> Fp;
	// 선형 보간된 위치를 계산.
	Fp = GetTimeFraction(Btt.PositionTimeStamps, DeltaTime);
	Vector3 Position1 = Btt.Positions[Fp.first - 1];
	Vector3 Position2 = Btt.Positions[Fp.first];
	Vector3 Position = FMath::Lerp(Position1, Position2, Fp.second);

	Fp = GetTimeFraction(Btt.RotationTimeStamps, DeltaTime);
	Quaternion _Rotation1 = Btt.Rotations[Fp.first - 1];
	Quaternion _Rotation2 = Btt.Rotations[Fp.first];
	Quaternion _Rotation = FMath::SLerp(_Rotation1, _Rotation2, Fp.second);

	Fp = GetTimeFraction(Btt.ScalTimeStamps, DeltaTime);
	Vector3 Scale1 = Btt.Scales[Fp.first - 1];
	Vector3 Scale2 = Btt.Scales[Fp.first];
	Vector3 Scale = FMath::Lerp(Scale1, Scale2, Fp.second);

	Matrix PositionMat = FMath::Translation(Position);
	Matrix RotationMat = FMath::Rotation(_Rotation);
	Matrix ScaleMat = FMath::Scale(Scale);
	// 보간된 애니메이션 행렬
	Matrix LocalTransform = ScaleMat * RotationMat * PositionMat;
	Matrix GlobalTransform = ParentTransform * LocalTransform;

	Output[Skeleton.ID] = GlobalInverseTransform * GlobalTransform * Skeleton.Offset;
	for (Bone& Child : Skeleton.Children)
	{
		GetPose(_Animation, Child, DeltaTime, Output, GlobalTransform, GlobalInverseTransform);
	}
	std::cout << DeltaTime << " => " << Position.x << ":" << Position.y << ":" << Position.z << ":" << std::endl;
}



//aiProcess_JoinIdenticalVertices // 동일한 꼭지점 결합, 인덱싱 최적화
//aiProcess_ValidateDataStructure // 로더의 출력을 검증
//aiProcess_ImproveCacheLocality// 출력 정점의 캐쉬위치를 개선
//aiProcess_RemoveRedundantMaterials // 중복된 매터리얼 제거
//aiProcess_GenUVCoords // 구형, 원통형, 상자 및 평면 매핑을 적절한 UV로 변환
//aiProcess_TransformUVCoords // UV 변환 처리기 (스케일링, 변환...)
//aiProcess_FindInstances // 인스턴스된 매쉬를 검색하여 하나의 마스터에 대한 참조로 제거
//aiProcess_LimitBoneWeights// 정점당 뼈의 가중치를 최대 4개로 제한
//aiProcess_OptimizeMeshes// 가능한 경우 작은 매쉬를 조인
//aiProcess_GenSmoothNormals// 부드러운 노말벡터(법선벡터) 생성
//aiProcess_SplitLargeMeshes// 거대한 하나의 매쉬를 하위매쉬들로 분활(나눔)
//aiProcess_Triangulate// 3개 이상의 모서리를 가진 다각형 면을 삼각형으로 만듬(나눔)
//aiProcess_ConvertToLeftHanded// D3D의 왼손좌표계로 변환
//aiProcess_SortByPType// 단일타입의 프리미티브로 구성된 '깨끗한' 매쉬를 만듬
//aiProcess_GenNormals // 모델 정보에 노말이 없을 경우 노말 생성한다. 
//aiProcess_CalcTangentSpace// 모델 정보에 탄젠트와 바이탄젠트가 없을경우 생성
//aiProcess_JoinIdenticalVertices // 중복 정점 하나로 합치기

void Engine::Model::LoadModel(const std::filesystem::path& Path, const std::filesystem::path& Name ,
	IDirect3DDevice9* const Device)&
{
	Assimp::Importer AssimpImporter{};

	auto ModelScene = AssimpImporter.ReadFile( (Path/Name).string(),
		aiProcess_JoinIdenticalVertices        |
		aiProcess_ValidateDataStructure 	   |
		aiProcess_ImproveCacheLocality		   |
		aiProcess_RemoveRedundantMaterials 	   |
		aiProcess_GenUVCoords 				   |
		aiProcess_TransformUVCoords			   |
		aiProcess_FindInstances 			   |
		aiProcess_LimitBoneWeights			   |
		aiProcess_OptimizeMeshes			   |
		aiProcess_GenSmoothNormals			   |
		aiProcess_SplitLargeMeshes			   |
		aiProcess_Triangulate				   |
		aiProcess_ConvertToLeftHanded		   |
		aiProcess_SortByPType				   |
		aiProcess_GenNormals 				   |
		aiProcess_CalcTangentSpace			   |
		aiProcess_JoinIdenticalVertices		   
		); 

	ModelScene->mRootNode->mTransformation;



	/*aiMesh* Mesh = ModelScene->mMeshes[0u];

	std::vector<Vertex::Animation> Vertices = {};
	std::vector<uint32> Indices = {};
	uint32 BoneCount = 0u;
	Animation _Animation;
	uint32 vao = 0u;
	Bone Skeleton;
	uint32 DiffuseTexture;

	Matrix GlobalInverseTransform = AssimpTo(ModelScene->mRootNode->mTransformation);
	GlobalInverseTransform  = FMath::Inverse(GlobalInverseTransform);

	LOAD_MODEL(ModelScene, Mesh, Vertices, Indices, Skeleton, BoneCount);
	LoadAnimation(ModelScene, _Animation);

	Device->CreateVertexBuffer(
		sizeof(Vertex::Animation) * Vertices.size(),
		D3DUSAGE_DYNAMIC, NULL, D3DPOOL_DEFAULT, &VertexBuffer, nullptr);
	Vertex::Animation* VertexBufferPtr{}; 
	VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBuffer), NULL);
	std::memcpy(VertexBufferPtr, Vertices.data(), sizeof(Vertex::Animation) * Vertices.size());
	VertexBuffer->Unlock();*/

	
	/*std::vector<Vertex::Texture> Vertices;
	uint32 CountAllVertices = 0u;
	
	for (uint32 MeshIdx = 0u; MeshIdx < ModelScene->mNumMeshes; ++MeshIdx)
	{
		auto& CurretMesh = ModelScene->mMeshes[MeshIdx];
		const uint32 NumVertices = CurretMesh->mNumVertices;
		CountAllFacices +=CurretMesh->mNumFaces;
		for(uint32 VertexIdx = 0u; VertexIdx < NumVertices; ++VertexIdx)
		{
			Vertex::Texture CurrentVertex;
			Vector3 Location, Normal; Vector2 UV;

			std::memcpy(&Location,&CurretMesh->mVertices[VertexIdx],
				sizeof(Vector3));
			std::memcpy(&Normal, &CurretMesh->mNormals[VertexIdx],
				sizeof(Vector3));
			std::memcpy(&UV, &CurretMesh->mTextureCoords[VertexIdx],
				sizeof(Vector2));

			Vertices.push_back(Vertex::Texture
				{
					std::move(Location), 
					std::move(Normal),
					std::move(UV) 
				} );
		}
		CountAllVertices += NumVertices;
	}
	
	Device->CreateVertexBuffer(sizeof(Vertex::Texture) * CountAllVertices,
		D3DUSAGE_DYNAMIC, Vertex::Texture::FVF, D3DPOOL_DEFAULT, &VertexBuffer,
		nullptr);
	auto& ResourceSys = ResourceSystem::Instance;
	;
	ResourceSys->Insert<IDirect3DVertexBuffer9>(L"VertexBuffer" + Name.generic_wstring(), VertexBuffer);

	decltype(Vertices)::value_type*  VertexBufferPtr{};
	VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBuffer), NULL);

	std::memcpy(VertexBufferPtr, Vertices.data(),
	sizeof( decltype(Vertices)::value_type )* Vertices.size());

	VertexBuffer->Unlock();*/
}

void Engine::Model::Render(IDirect3DDevice9* Device)&
{
	//Device->SetFVF(Vertex::Texture::FVF);
	//Device->SetStreamSource(0, VertexBuffer, 0, sizeof(Vertex::Texture));
	//Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0u, CountAllFacices);
	//Device->SetVertexShader(nullptr);
	//Device->SetPixelShader(nullptr);
}


