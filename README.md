# 3DGame

1. 렌더 컴포넌트는 업데이트시 렌더러에게 정보를 주기.
2. 렌더러가 렌더링 수행

## 리소스
https://gall.dcinside.com/mgallery/board/view/?id=kurtzpel&no=5904
https://gall.dcinside.com/mgallery/board/view/?id=kurtzpel&no=5661&exception_mode=recommend&page=1
https://store.steampowered.com/app/844870/KurtzPel/?l=koreana
https://gall.dcinside.com/mgallery/board/view/?id=lovebeat&no=319
https://gall.dcinside.com/mgallery/board/view/?id=kurtzpel&no=6898&page=1


# Skin
본 업데이트시 최상위 루트부터 시작해서
하위 노드 자식들에게 월드행렬 전파
재귀로 순회하며  자식노드들 행렬 업데이트

책을 봐도 이해가 안갔다 뼈대 애니를 그대로 곱한다니... 스킨의 로컬 정점에 다가...

결국 알아낸 진짜 계산식...

V(최종 world) = V(스킨 world) * M0(월드 상에서 뼈가 움직인 만큼의 애니) * W0(가중치) ....

그럼 저 M0는 어떻게 구하느냐...

일단 뼈의 움직임을 월드 상의 움직임으로 바꾸어야 한다.

뼈 AniTm을 그대로 적용하면 뼈가 A만큼 움직였다고 해서 스킨도 A만큼 움직이지지 않는다.

왜냐?

뼈 AniTm은 뼈의 로컬 정점이 월드에 적용되는 AniTm 값이기 때문에 이를 그대로 스킨의 로컬 정점에 적용하면 각 메시들의 변환 행렬의 차이로 다른 움직임을 보인다.그렇기 때문에 월드 상에서 뼈가 움직인 AniTm만큼 월드 상에 스킨 정점을 이동하는 방법을 찾아야 한다.

월드 상에서 시작 기준이 될 행렬 OffsetTm을 구한다

OffsetTm = 뼈의 초기 월드 행렬(움직임이 없는 고정된 상태에서 계산된 뼈 월드 행렬)의 역행렬

월드 상에서 뼈의 움직임 값을 구한다

(월드 상의 뼈 움직임) = OffsetTm * Bone AniTm

이를 월드 상의 정점에 적용

V(최종 world) = V(스킨 world) * (월드 상의 뼈 움직임)

스킨 로컬 상에서 뼈의 로컬 상의 움직임만큼 스킨 정점을 이동 후 월드화 해주는 방법도 있다.

아마 책에서 말했던 위의 식에서 말하는 뼈대 애니란 로컬 상에서의 뼈의 움직임을 말하는 것일 듯...

그럼 그렇게 적어놔야지 뼈대 애니 행렬이라고만 적으면 어떻하냐

## 일단 생각한 방법 1.
BoneStartWorld (뼈의 초기 월드 행렬 , 움직임이 없는 고정된 상태의 뼈의 월드 행렬)
본의 로컬에 오브젝트의 월드행렬을 곱했을 것이다. 
BoneStartWorld-1 
VWorld (스킨를 구성하는 버텍스가 로컬에서 오브젝트의 월드행렬로 변환됨)
VWorld * BoneStartWorld-1 = VLocal_in_Bone (버텍스를 똑같은 오브젝트의 월드공간에서 본의 로컬 공간으로 변환)
VLocal_in_Bone * BoneAnimTransform  * Weight = VAnimWorld (본의 로컬공간의 버텍스를 본이 애니메이션을 위해 이동한 만큼 트랜스폼) 이후 가중치 적용
## 2. 
본과 버텍스 둘 다  모델의 로컬 좌표계 기준
v_local *( bone_local-1 ) = v_local_in_bone (버텍스를 본의 로컬공간으로 이동)
v_local_in_bone * animation_transform = (본의 로컬에 존재하는 버텍스를 본이 모델 좌표계에서  애니메이션 한만큼 트랜스폼)
v_final = v_local_in_bone * object_world

## 3.
v_world = v_local(스킨의 모델공간 로컬좌표) * object_world (오브젝트 월드좌표계로 변환)
* inverse_bone_world ( 본의 모델공간 로컬좌표 * 오브젝트 월드좌표 변환 이후의 역행렬 )
* bone_animation_in_world (본의 애니메이션 월드행렬) * weight[targetboneindex]

## 4.
VWorld  * BoneWorld-1  = v_bone_world_local

v_bone_world_local * bone_world_animation  = v_final
## 5.
v_local * (b_local-1) * b_local_anim
* b_world
(모델 공간에서의 버텍스와 본으로부터 시작)
1. 버텍스를 본 기준 좌표계로 트랜스폼
2. 본이 초기위치에서 애니메이션 된 만큼만 똑같이 변환
3. 이후의 본의 월드공간으로 변환
(본의 월드공간은 본의 로컬 * 모델을 사용하는 오브젝트의 월드)
# Skeleton 

* 애니메이션 행렬은 본의 원래 위치에서
변환이 끝난 오프셋 행렬
* 모델 공간에서 만들어진 버텍스를 
본의 로컬공간에서의 위치를 뜻하는 행렬을 곱해서 본 기준 좌표계로 옮김.


교점  =( (구체중심 - 광선시작점 ) dot 광선 방향 ) 광선방향 + 광선 시작점
if | 교점 - 구체 중심 | < 구체 반지름 ? 충돌 !
else 충돌 아님

# 3ds max 애니메이션
1. fbx load
2. 모든 계층 구조 선택 Ctrl+A
3. 애니메이션 저장 (저장 할시 키프레임 시작지점 ~ 끝지점 매칭)
4. 내가 이어 붙일 시점으로 키프레임 이동한 이후에 애니메이션 로딩

애니메이션 클립 수정 까먹지 말기





