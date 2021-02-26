
# 3DGame

1. 렌더 컴포넌트는 업데이트시 렌더러에게 정보를 주기.
2. 렌더러가 렌더링 수행

## 리소스
https://gall.dcinside.com/mgallery/board/view/?id=kurtzpel&no=5904
https://gall.dcinside.com/mgallery/board/view/?id=kurtzpel&no=5661&exception_mode=recommend&page=1
https://store.steampowered.com/app/844870/KurtzPel/?l=koreana
https://gall.dcinside.com/mgallery/board/view/?id=lovebeat&no=319
https://gall.dcinside.com/mgallery/board/view/?id=kurtzpel&no=6898&page=1


교점  =( (구체중심 - 광선시작점 ) dot 광선 방향 ) 광선방향 + 광선 시작점
if | 교점 - 구체 중심 | < 구체 반지름 ? 충돌 !
else 충돌 아님

# 3ds max 애니메이션
1. fbx load
2. 모든 계층 구조 선택 Ctrl+A
3. 애니메이션 저장 (저장 할시 키프레임 시작지점 ~ 끝지점 매칭)
4. 내가 이어 붙일 시점으로 키프레임 이동한 이후에 애니메이션 로딩

애니메이션 클립 수정 까먹지 말기



# Assimp
## 라이브러리 정보
***Scene 정보를 로딩시 옵션중 호환이 안되는 옵션들을 사용하거나 같은 옵션을 중복 사용하면  undefined behavior 이며 에러 핸들링에 관련된  기능이 딱히 없는것 같으므로 유의해서 코드 작성하는 수밖에 없음.***
* 본의 Transform 이 모델링 정보와 일치하지 않는 경우가 있는데 Assimp가 비율을 축소하는 경우가 있는것 같다.
 (ex Bone Location : 12,4,2 -> Assimp Load Bone Location : 6,2,1)
 * Scene 정보중 쓸데없는 값들 예를 들어 카메라 위치라거나 광원위치가 저절로 삽입 되었고 인지하지 못하고 사용할 경우 Root Node 로부터 순회할때 자식으로 매칭 되므로 ***진짜 매우 매우*** 짜증난다 간혹 이상하게 애니메이션 되는 경우 반드시 확인할것.
 (반대로 이 정보를 활용해 많은 걸 할 수 있을것 같다.)
 * Assimp Node == Bone
 * Bone.Transform == Bone Local Space To Parent Space
 * Root 의 Transform 이 모델링의 기저와 일치하지 않거나 포지션 이동 하였을 경우 당연하지만
 Bind Space 와 모델의 Local Space 는 일치하지 않는다. 그 이외는 일치.
 * Assimp 에서 사용하는 좌표축은 OpenGL과 동일한 오른손 좌표계이며 열기준 행렬이므로 directx 의 왼손 좌표계로 데이터를 읽어들이도록 ReadFile 사용시
 몇가지 옵션을 지정해주어야 하며 행렬 사용시 전치해주어야 함.
  ## 자주쓰일 법한 로딩 옵션
aiProcess_JoinIdenticalVertices // 동일한 꼭지점 결합, 인덱싱 최적화 중복 정점 하나로 합치기
aiProcess_ValidateDataStructure // 로더의 출력을 검증
aiProcess_ImproveCacheLocality// 출력 정점의 캐쉬위치를 개선
aiProcess_RemoveRedundantMaterials // 중복된 매터리얼 제거
aiProcess_GenUVCoords // 구형, 원통형, 상자 및 평면 매핑을 적절한 UV로 변환
aiProcess_TransformUVCoords // UV 변환 처리기 (스케일링, 변환...)
aiProcess_FindInstances // 인스턴스된 매쉬를 검색하여 하나의 마스터에 대한 참조로 제거
aiProcess_LimitBoneWeights// 정점당 뼈의 가중치를 최대 4개로 제한
aiProcess_OptimizeMeshes// 가능한 경우 작은 매쉬를 조인
aiProcess_GenSmoothNormals// 부드러운 노말벡터(법선벡터) 생성
aiProcess_SplitLargeMeshes// 거대한 하나의 매쉬를 하위매쉬들로 분활(나눔)
aiProcess_Triangulate// 3개 이상의 모서리를 가진 다각형 면을 삼각형으로 만듬(나눔)
aiProcess_ConvertToLeftHanded// D3D의 왼손좌표계로 변환
aiProcess_SortByPType// 단일타입의 프리미티브로 구성된 '깨끗한' 매쉬를 만듬
aiProcess_GenNormals // 모델 정보에 노말이 없을 경우 노말 생성한다.
aiProcess_CalcTangentSpace// 모델 정보에 탄젠트와 바이탄젠트가 없을경우 생성

## 스켈레톤 애니메이션
* 본의 트랜스폼 매트릭스는 바로 위의 부모 공간에서 해당 본의 공간으로 좌표계를 변환한다. **(SRT * Parent World) 를 떠올리자.**
$$Bone Transform Matrix = B$$

* 본의 애니메이션 매트릭스는 (키프레임과 키프레임을 보간해서 구한 새로운 트랜스폼) 부모 공간에서 해당 본이 해당 애니메이션에서 해당 시간이 흘렀을때의 공간으로 좌표계를 변환한다.
**(트랜스폼 매트릭스가 바뀌었을 뿐)**
$$Is Animation ?  B = Animation Transform$$
* 본의 오프셋 매트릭스는 해당 본과 모델링 스페이스를 공유하는 정점의 위치를 해당 본의 상대적인 좌표계로 변환한다.
***(같은 월드 좌표계를 공유하는 카메라의 상태를 정의하는 행렬의 역행렬이 뷰 매트릭스 임을 떠올리자)***
* 위의 사실로 미루어 볼때 어떠한 본을 루트공간 좌표계로 변환하는 행렬을 ToRoot 라고 한다면 ToRoot는 다음과 같다.
해당 본의 트랜스폼  과 바로 위 부모의 트랜스폼행렬의 곱셈이다. (여기서 부모의 ToRoot는 당연히 부모본을 루트공간으로 변환하는 행렬이다. )
$$ToRoot = BoneTransform * ParentToRoot$$
* 본의 ToRoot 계산을 위해서는  ParentToRoot 는 계산이 끝나있는것이 가장 베스트이며 그렇다면 Root 본부터 시작하여 트리를 깊이우선이든 너비우선이든 하향식으로 운행하는 것이 가장 효율적임을 알 수 있다.
* 본과 모델링 공간을 공유하는 정점이 해당 본이 애니메이션이 끝났을때의 좌표변환을 의미하는 행렬은 다음과 같다.
$$ AfterAnimationForVertex = Bone Offset * ToRoot$$
* 스키닝 애니메이션을 위해서는 정점마다 참조하는 본의 개수와 가중치(모두 더해서 당연히 1이 나와야 한다.) 정보를 이용해서 AfterAnimationForVertex 와 곱한 좌표에 해당하는 가중치만큼 곱하는 연산을 반복하며 모두 더하면 되겠다.

### 유의 할 점
* 쿼터니언 보간은 구면 선형 보간을 사용해라 (별로 안 날줄 알았는데 차이가 꽤 큼)

## TODO ::
* 스카이 스피어 문제 해결해야함 .
* 부유물체들 운동
* 공중 떠있는 길 간단한 물리
* 플레이어 애니메이션 편집한다음 고고
## 디퍼드 쉐이딩
1. 렌더타겟의 초기화는 디바이스가 해야한다.
2. 렌더타겟의 클리어 단계는 기존 바인딩 렌더타겟 해제 -> 클리어 하려는 렌더타겟 바인딩 이후의 클리어이다.
3. 렌더링 루프 진입 직전 모든 렌더타겟 클리어 먼저하고 진입하면 깔끔하겠다.
4. 렌더타겟 획득시 당연히도 레퍼런스 카운트가 증가하지만 아웃풋 매개변수라 눈치 못챌수도 있어서 적어놓음.
5. 디버그 렌더타겟 출력시 동차좌표계 4D벡터로 할수도 있더라 편리해보임 (XYZRHW)
6. DX9 API 기준 픽셀 쉐이더에서 한번의 패스에서 출력 할수 있는 픽셀 개수는 4개다 (COLOR0, COLOR1,COLOR2,COLOR3)
7. 렌더타겟 노말 저장 단계에서 RGB로 패킹할때 0~1로 범위 축소 시키는거 실수하지 마라 ( -1 ~ 1 => 0 ~ 1)
8. 텍스쳐 샘플링 당연하게도 보간하면 안된다. 실수하지마라
9. 픽셀 값이 뭔가 미묘하게 밀리는듯 하다면 적절한 바이어스를 추가하자 (ex +0.5) 매직넘버
10.RGB에서 노말 언패킹 할때도 좌표변환 잊지말자 (0~1 => -1 ~ 1)
## Depth Shadow Mapping
* 퍼센티지 클로저로 깔쌈하게 출력해보자.
* 그림자 정교하게 하고싶다면 텍스쳐 크기를 늘리는걸 고려하자.
* 좌표계를 넓게 잡고 있다면 바이어스에 계수를 곱하는걸 고려해라.
