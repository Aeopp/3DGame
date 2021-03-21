#pragma once
#include "Object.h"
#include "SkeletonMesh.h"

class NPC  final : public Engine::Object
{
public:
	using Super = Engine::Object;
	using MyType = NPC;

	enum class InteractionEvent : uint8
	{
		FirstEncounter,
		SecondEncounter,
		ThirdEncounter,
		End,
	};

	struct EventMentInformation
	{
		std::wstring FontName{ L"Font_Sandoll" };
		EventMentInformation(
			const std::wstring& Ment,
			const Vector2& Position,
			const D3DXCOLOR& _Color)  :
			Ment{ Ment }, Position{ Position } ,Color{ _Color }{};

		std::wstring Ment{L"NPC ¿« ¥Î»≠ ∏‡∆Æ"}; 
		Vector2 Position{ 0.f,0.f };
		D3DXCOLOR Color = {1.f,1.f,1.f,1.f};
		/*RefFontManager().RenderRegist(L"Font_Sandoll", L"NPC ¿« ¥Î»≠ ∏‡∆Æ.",
			{ -300,-300 }, D3DXCOLOR{ 1.f,1.f,1.f,1.f });*/
	};
public:
	void Initialize(const std::optional<Vector3>& Scale, const std::optional<Vector3>& Rotation, const Vector3& SpawnLocation)&;
	void PrototypeInitialize(IDirect3DDevice9* const Device)&;
	virtual std::shared_ptr<Engine::Object> GetCopyShared()& override;
	virtual std::optional<SpawnReturnValue>
		InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam)& override;
	virtual std::function< SpawnReturnValue(const SpawnParam&)>
		PrototypeEdit()& override ;
	virtual void Event()&override;
	virtual void Update(const float DeltaTime)&;
	virtual void LateUpdate(const float DeltaTime)&;

	void Edit()&;

	virtual void HitNotify(Object* const Target,
		const Vector3 PushDir, const float CrossAreaScale) & override;

	void NextInteraction()&;
	void NextEvent()&;

	Vector3 ViewLocationOffset{ 0,15.929f, -5.310f};

	NPC::InteractionEvent CurrentEvent = NPC::InteractionEvent::FirstEncounter;
	uint32                CurrentEventIndex = 0u;
	std::map <NPC::InteractionEvent, std::vector<EventMentInformation>> EventMents{};

	bool bTutorialWaveStart = false;
	bool bInteraction = false;
private:
	IDirect3DDevice9* Device{ nullptr };
};

