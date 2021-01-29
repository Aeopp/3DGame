#include "Model.h"
#include "ResourceSystem.h"


// assimp Node 클래스의 정보를 참조해서 로딩이 완료되었다고
// 가정하는 본 정보 테이블에서 타겟 본 정보를 초기화해서 넘겨준다.
bool Engine::Model::ReadSkeleton(Bone& BoneOutput,aiNode* Node, std::unordered_map<std::string, 
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

void Engine::Model::LOAD_MODEL(const aiScene* Scene, aiMesh* Mesh, std::vector<Vertex::Animation>& VerticesOutput, std::vector<uint32>& IndicesOutput, Bone& SkeletonOutput, uint32& BoneCount)
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
	//                    본 이름          본 아이디   
	std::unordered_map<std::string, std::pair<int32,Matrix>> BoneInfo{};
	// 배열의 인덱스는 버텍스의 아이디 원소는 참조하는 본의(최대4개) 인덱스
	std::vector<uint32>BoneCounts{};
	BoneCounts.resize(VerticesOutput.size(), 0);
	BoneCount = Mesh->mNumBones;
	
	// 본을 전부 순회.
	for (uint32 i = 0; i < BoneCount; ++i)
	{
		// 오프셋 매트릭스를 저장. 
		aiBone* bone = Mesh->mBones[i];
		Matrix _Matrix;
		std::memcpy(&_Matrix, &bone->mOffsetMatrix, sizeof(Matrix));
		
	}

	
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

