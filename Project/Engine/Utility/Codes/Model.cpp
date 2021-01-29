#include "Model.h"
#include "ResourceSystem.h"
#include <iostream>


// assimp Node Ŭ������ ������ �����ؼ� �ε��� �Ϸ�Ǿ��ٰ�
// �����ϴ� �� ���� ���̺��� Ÿ�� �� ������ �ʱ�ȭ�ؼ� �Ѱ��ش�.
bool Engine::Model::ReadSkeleton(Bone& BoneOutput,aiNode* Node, 
	//���������̺��� �޽����� ������ ���� .
	std::unordered_map<std::string,  /*��� �޽��� �����ϴ� assimp �� �̸�*/
	std::pair<int32/*�� ���̵�*/, /*�� ������ ��Ʈ����(??)*/Matrix>>& BoneInfoTable)
{
	// �� ���̺��� ������ ã�µ� ���� !!
	if (BoneInfoTable.find(Node->mName.C_Str()) != BoneInfoTable.end())
	{
		// �� ������ ���� ä�� �ִ´�. 
		BoneOutput.Name = Node->mName.C_Str();
		BoneOutput.ID = BoneInfoTable[BoneOutput.Name].first;
		BoneOutput.Offset = BoneInfoTable[BoneOutput.Name].second;

		for (int32 i = 0; i < Node->mNumChildren; ++i)
		{
			// Node �� �ڽĵ���� ���� ��ͷ� ��ȸ�ϸ� �ڽ� ��������
			// ä���ִ´�. assimp �� �о�� Ʈ�� ������
			// ����� ���� ����ü Bone ���� �Ȱ��� ��Ī ���� �ִ� ���̴� . 
			Bone Child;
			ReadSkeleton(Child, Node->mChildren[i], BoneInfoTable);
			BoneOutput.Children.push_back(Child);
		}
		return true;
	}
	else // ����� �̸����� ã�� ���� (�� ���̺� ������ ���� ��Ȳ.)
	{
		// �ش� ����� �ڽĵ鿡�Լ� ������ ã�´�. (�־��� ��� Leaf ���� ��������.)
		for (int i = 0; i < Node->mNumChildren; ++i)
		{
			if (ReadSkeleton(BoneOutput, Node->mChildren[i], BoneInfoTable))
			{
				return true;
			}
		}
	}

	// Leaf ���� ���������� ������ ã�� ���Ͽ���.
	return false;
}

void Engine::Model::LOAD_MODEL(const aiScene* Scene, aiMesh* Mesh, std::vector<Vertex::Animation>& VerticesOutput,
	std::vector<uint32>& IndicesOutput, 
	Bone& SkeletonOutput, uint32& BoneCount)
{
	VerticesOutput = {};
	IndicesOutput = {};
	// �޽��� ������ �ε�
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
		// �ؽ��� ��ǥ�� ������ ���ٸ� �ε��� ����.
		_Vector2.x = Mesh->mTextureCoords[0][i].x;
		_Vector2.y = Mesh->mTextureCoords[0][i].y;
		_Vertex.UV = _Vector2;

		_Vertex.BoneIds = Vector4{0,0,0,0};
		_Vertex.BoneWeights = Vector4{0,0,0,0};

		VerticesOutput.push_back(_Vertex);
	}

	// �� ������ �ε�
	//               ��� �޽��� �����ϴ� �� (assimp) �̸�          �� ���̵�   
	//���������̺��� �޽����� ������ ���� . 
	std::unordered_map<std::string, std::pair<int32,Matrix>> BoneInfo{};
	// �迭�� �ε����� ���ؽ��� ���̵� ���Ҵ� �����ϴ� ����(�ִ�4��) ����
	std::vector<uint32>BoneCounts{};
	BoneCounts.resize(VerticesOutput.size(), 0u);
	BoneCount = Mesh->mNumBones;
	
	// ���� ���� ��ȸ.
	for (uint32 i = 0u; i < BoneCount; ++i)
	{
		// ������ ��Ʈ������ ����. 
		aiBone* bone = Mesh->mBones[i];
		Matrix _Matrix;
		                    
		std::memcpy(&_Matrix, &bone->mOffsetMatrix
			/*���ε������� �޽ð������� ��� �������� ��ȯ*/
			, sizeof(Matrix));
		BoneInfo[bone->mName.C_Str()] = { i, _Matrix};

		// �ش� ���� ����ġ ������ŭ ��ȸ
		for (int32 j = 0; j < bone->mNumWeights; ++j)
		{
			// ���� ������ �ִ� ���ؽ� ���̵�� ���εǾ��ִ�
			// ����ġ ������ �����ؼ� 
			// ���ؽ����� ����ġ�� �� ���� �ε��� ������ ä�� �ִ´�.  
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

	// ����ġ�� ����ȭ �ϸ� ����ġ�� ���� �ݵ�� 1�� �ǵ��� ������ �����ϰ� �����Ѵ�.
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

	// �޼� ��ǥ�� �ɼ����� �ε� �Ͽ��ٸ�
	// ���̽��� �����ϴ� 0,1,2 ���� �ﰢ���� �׸��� ������
	// ��ġ�ϹǷ� �ش� ������ �ε��� ������ ä�� �ִ´�.
	for (int32 i = 0; i < Mesh->mNumFaces; ++i)
	{
		aiFace& _Face = Mesh->mFaces[i];
		for (uint32 j = 0; j < _Face.mNumIndices; ++j)
		{
			IndicesOutput.push_back(_Face.mIndices[j]);
		}
	}
	
	// �� ���̺� ������ ä���� ���Ŀ� ���̷��� ������ �ε��Ѵ�. ���� ��Ʈ���κ���.
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
	// �� ���� ���� ��ġ ȸ�� �����ϸ� �ε�
	// ä�� = ��
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
	// ���� ������ ��ġ�� ���.
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
	// ������ �ִϸ��̼� ���
	Matrix LocalTransform = ScaleMat * RotationMat * PositionMat;
	Matrix GlobalTransform = ParentTransform * LocalTransform;

	Output[Skeleton.ID] = GlobalInverseTransform * GlobalTransform * Skeleton.Offset;
	for (Bone& Child : Skeleton.Children)
	{
		GetPose(_Animation, Child, DeltaTime, Output, GlobalTransform, GlobalInverseTransform);
	}
	std::cout << DeltaTime << " => " << Position.x << ":" << Position.y << ":" << Position.z << ":" << std::endl;
}



//aiProcess_JoinIdenticalVertices // ������ ������ ����, �ε��� ����ȭ
//aiProcess_ValidateDataStructure // �δ��� ����� ����
//aiProcess_ImproveCacheLocality// ��� ������ ĳ����ġ�� ����
//aiProcess_RemoveRedundantMaterials // �ߺ��� ���͸��� ����
//aiProcess_GenUVCoords // ����, ������, ���� �� ��� ������ ������ UV�� ��ȯ
//aiProcess_TransformUVCoords // UV ��ȯ ó���� (�����ϸ�, ��ȯ...)
//aiProcess_FindInstances // �ν��Ͻ��� �Ž��� �˻��Ͽ� �ϳ��� �����Ϳ� ���� ������ ����
//aiProcess_LimitBoneWeights// ������ ���� ����ġ�� �ִ� 4���� ����
//aiProcess_OptimizeMeshes// ������ ��� ���� �Ž��� ����
//aiProcess_GenSmoothNormals// �ε巯�� �븻����(��������) ����
//aiProcess_SplitLargeMeshes// �Ŵ��� �ϳ��� �Ž��� �����Ž���� ��Ȱ(����)
//aiProcess_Triangulate// 3�� �̻��� �𼭸��� ���� �ٰ��� ���� �ﰢ������ ����(����)
//aiProcess_ConvertToLeftHanded// D3D�� �޼���ǥ��� ��ȯ
//aiProcess_SortByPType// ����Ÿ���� ������Ƽ��� ������ '������' �Ž��� ����
//aiProcess_GenNormals // �� ������ �븻�� ���� ��� �븻 �����Ѵ�. 
//aiProcess_CalcTangentSpace// �� ������ ź��Ʈ�� ����ź��Ʈ�� ������� ����
//aiProcess_JoinIdenticalVertices // �ߺ� ���� �ϳ��� ��ġ��

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


