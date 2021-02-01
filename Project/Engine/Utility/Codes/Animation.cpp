#include "Animation.h"

void Engine::Aniamtion::Initialize(ID3DXAnimationController* const AnimationControl)&
{
	this->AnimationControl= DX::SharedPtr<ID3DXAnimationController>(AnimationControl);
}

void Engine::Aniamtion::Clone()&
{
	ID3DXAnimationController* _Clone{ nullptr };

	AnimationControl->CloneAnimationController(
		AnimationControl->GetMaxNumAnimationOutputs(),
		AnimationControl->GetMaxNumAnimationSets(),
		AnimationControl->GetMaxNumTracks(),
		AnimationControl->GetMaxNumEvents() ,
		&_Clone);

	AnimationControl  = DX::MakeShared<ID3DXAnimationController>(_Clone);
}

void Engine::Aniamtion::SetAnimationIndex(const uint32 Index)&
{
	OriginIndex = Index;

	NewTrack = CurrentTrack == 0 ? 1 : 0;
	ID3DXAnimationSet* AnimSet = nullptr;

	// 애니메이션 셋이 진행되는 최종의 시간 값을 반환
	Duration = AnimSet->GetPeriod();

	m_pAniCtrl->SetTrackAnimationSet(m_iNewTrack, pAS);

	m_pAniCtrl->UnkeyAllTrackEvents(m_iCurrentTrack);
	m_pAniCtrl->UnkeyAllTrackEvents(m_iNewTrack);

	m_pAniCtrl->KeyTrackEnable(m_iCurrentTrack, FALSE, m_fAccTime + 0.25);
	m_pAniCtrl->KeyTrackSpeed(m_iCurrentTrack, 1.f, m_fAccTime, 0.25, D3DXTRANSITION_LINEAR);
	// 현재 구동하는 트랙의 가중치를 결정하는 함수
	m_pAniCtrl->KeyTrackWeight(m_iCurrentTrack, 0.1f, m_fAccTime, 0.25, D3DXTRANSITION_LINEAR);

	// 트랙을 활성화할지 결정하는 함수
	m_pAniCtrl->SetTrackEnable(m_iNewTrack, TRUE);
	m_pAniCtrl->KeyTrackSpeed(m_iNewTrack, 1.f, m_fAccTime, 0.25, D3DXTRANSITION_LINEAR);
	m_pAniCtrl->KeyTrackWeight(m_iNewTrack, 0.9f, m_fAccTime, 0.25, D3DXTRANSITION_LINEAR);

	m_pAniCtrl->ResetTime();	// 애니메이션이 재생될 때 증가는 시간값을 초기화하는 함수
	m_fAccTime = 0.f;

	m_pAniCtrl->SetTrackPosition(m_iNewTrack, 0.0);
	m_iOriginIdx = iIndex;

	m_iCurrentTrack = m_iNewTrack;
};
