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

void Engine::Aniamtion::Set(const uint32 Index)&
{
	OriginIndex = Index;

	NewTrack = CurrentTrack == 0 ? 1 : 0;
	ID3DXAnimationSet* AnimSet = nullptr;

	// �ִϸ��̼� ���� ����Ǵ� ������ �ð� ���� ��ȯ
	Duration = AnimSet->GetPeriod();

	AnimationControl->SetTrackAnimationSet(NewTrack, AnimSet);

	AnimationControl->UnkeyAllTrackEvents(CurrentTrack);
	AnimationControl->UnkeyAllTrackEvents(NewTrack);

	AnimationControl->KeyTrackEnable(CurrentTrack, FALSE, AccelerationTime + 0.25);
	AnimationControl->KeyTrackSpeed(CurrentTrack, 1.f, AccelerationTime,
		0.25, D3DXTRANSITION_LINEAR);
	// ���� �����ϴ� Ʈ���� ����ġ�� �����ϴ� �Լ�
	AnimationControl->KeyTrackWeight(CurrentTrack, 0.1f,
		AccelerationTime, 0.25, D3DXTRANSITION_LINEAR);

	// Ʈ���� Ȱ��ȭ���� �����ϴ� �Լ�
	AnimationControl->SetTrackEnable(NewTrack, TRUE);
	AnimationControl->KeyTrackSpeed(NewTrack, 1.f, AccelerationTime, 0.25, D3DXTRANSITION_LINEAR);
	AnimationControl->KeyTrackWeight(NewTrack, 0.9f, AccelerationTime, 0.25, D3DXTRANSITION_LINEAR);

	AnimationControl->ResetTime();	// �ִϸ��̼��� ����� �� ������ �ð����� �ʱ�ȭ�ϴ� �Լ�
	AccelerationTime = 0.f;

	AnimationControl->SetTrackPosition(NewTrack, 0.0);
	OriginIndex = Index;

	CurrentTrack = NewTrack;
};

void Engine::Aniamtion::Play(const float DeltaTime)&
{
	AnimationControl->AdvanceTime(DeltaTime, NULL);
	AccelerationTime += DeltaTime;
}
bool Engine::Aniamtion::IsEnd()&
{
	D3DXTRACK_DESC  TrackInfo{};
	ZeroMemory(&TrackInfo, sizeof(D3DXTRACK_DESC));

	AnimationControl->GetTrackDesc(CurrentTrack, &TrackInfo);

	if (TrackInfo.Position >= Duration - 0.1)
		return true;

	return false;
}
;
