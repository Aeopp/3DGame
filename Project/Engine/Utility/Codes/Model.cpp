#include "Model.h"
#include "ResourceSystem.h"


// assimp Node Ŭ������ ������ �����ؼ� �ε��� �Ϸ�Ǿ��ٰ�
// �����ϴ� �� ���� ���̺��� Ÿ�� �� ������ �ʱ�ȭ�ؼ� �Ѱ��ش�.
bool Engine::Model::ReadSkeleton(Bone& BoneOutput,aiNode* Node, std::unordered_map<std::string, 
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

void Engine::Model::LOAD_MODEL(const aiScene* Scene, aiMesh* Mesh, std::vector<Vertex::Animation>& VerticesOutput, std::vector<uint32>& IndicesOutput, Bone& SkeletonOutput, uint32& BoneCount)
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
	//                    �� �̸�          �� ���̵�   
	std::unordered_map<std::string, std::pair<int32,Matrix>> BoneInfo{};
	// �迭�� �ε����� ���ؽ��� ���̵� ���Ҵ� �����ϴ� ����(�ִ�4��) �ε���
	std::vector<uint32>BoneCounts{};
	BoneCounts.resize(VerticesOutput.size(), 0);
	BoneCount = Mesh->mNumBones;
	
	// ���� ���� ��ȸ.
	for (uint32 i = 0; i < BoneCount; ++i)
	{
		// ������ ��Ʈ������ ����. 
		aiBone* bone = Mesh->mBones[i];
		Matrix _Matrix;
		std::memcpy(&_Matrix, &bone->mOffsetMatrix, sizeof(Matrix));
		
	}

	
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

